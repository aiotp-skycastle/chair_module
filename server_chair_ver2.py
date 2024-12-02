import serial
import socket
import requests
import time
import socket
requests.packages.urllib3.util.connection.allowed_gai_family = lambda: socket.AF_INET

# 1. Serial 통신 설정
arduino_port = '/dev/ttyACM0'
baud_rate = 115200
ser = serial.Serial(arduino_port, baud_rate)

# 2. HTTP 서버 설정
server_url = "https://skycastle.cho0h5.org/chair/status"

# 3. 책상 모듈 소켓 통신 설정
desk_host = "skycastle-desk.cho0h5.org"  # 책상 모듈 도메인 주소
desk_port = 65432

# 4. 로그 파일 설정
log_file = "arduino_data.log"

# 로그 기록 함수
def log_to_file(message):
    with open(log_file, "a") as file:
        file.write(f"{time.strftime('%Y-%m-%d %H:%M:%S')} - {message}\n")

# 서버로 데이터 전송 함수
def send_to_server(data):
    try:
        response = requests.post(server_url, json=data)
        if response.status_code == 200:
            print("Data sent to server successfully.")
            log_to_file(f"Server Response: {response.status_code}")
        else:
            print(f"Failed to send to server: {response.status_code}, Response: {response.text}")
            log_to_file(f"Server Error: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"Error sending to server: {e}")
        log_to_file(f"Server Exception: {e}")

# 책상 모듈 소켓 통신 함수
def send_to_desk_module(data):
    while True:
        try:
            # DNS 조회 및 소켓 연결
            ip_address = socket.gethostbyname(desk_host)
            print(f"도메인 {desk_host}의 IP 주소: {ip_address}")
            
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect((desk_host, desk_port))
            print(f"책상 모듈 {desk_host}:{desk_port}에 연결되었습니다.")
            
            # 데이터 전송
            client_socket.send(data.encode())
            print(f"전송한 데이터: {data}")
            
            # 응답 수신
            response = client_socket.recv(1024).decode()
            print(f"책상 모듈 응답: {response}")
            
            client_socket.close()
            return response
        except socket.gaierror as e:
            print(f"도메인 조회 실패: {e}. 5초 후 재시도...")
            time.sleep(5)
        except ConnectionRefusedError:
            print("책상 모듈과 연결할 수 없습니다. 5초 후 재시도...")
            time.sleep(5)
        except Exception as e:
            print(f"책상 모듈 통신 중 에러 발생: {e}. 5초 후 재시도...")
            time.sleep(5)

# 데이터 처리 함수
def process_data():
    while True:
        try:
            if ser.in_waiting > 0:
                # 아두이노로부터 데이터 읽기
                raw_data = ser.readline().decode('utf-8').strip()
                print(f"Received from Arduino: {raw_data}")
                log_to_file(f"Arduino Raw Data: {raw_data}")

                # 데이터 파싱
                sensor_values = raw_data.split(',')
                if len(sensor_values) == 3:
                    # 데이터 변환
                    button_count = int(sensor_values[0])
                    temperature = float(sensor_values[1])
                    distance = float(sensor_values[2])

                    # 상태 결정 조건
                    status = "on" if button_count >= 3 and distance <= 5 else "off"

                    # 서버 전송 데이터
                    server_data = {
                        "status": status
                    }

                    # 책상 모듈 전송 데이터
                    desk_data = f"{status},{button_count},{temperature},{distance}"

                    # 서버로 전송
                    send_to_server(server_data)

                    # 책상 모듈로 전송
                    send_to_desk_module(desk_data)

        except ValueError as e:
            print(f"ValueError: {e} - Raw Data: {raw_data}")
            log_to_file(f"ValueError: {e} - Raw Data: {raw_data}")
        except Exception as e:
            print(f"Error processing data: {e}")
            log_to_file(f"Processing Exception: {e}")
            break

# 메인 실행
if __name__ == "__main__":
    try:
        process_data()
    except KeyboardInterrupt:
        print("Stopping...")
    finally:
        ser.close()
