import tkinter as tk
from tkinter import scrolledtext, simpledialog
import socket
import threading
import struct

# 닉네임을 전역 변수로 설정
nickname = ""
is_muted = False
count = 0
first_client = False
NOT_LAST_Client = False

#recv()
def receive_messages():
    message = ""  # 수신한 데이터를 저장할 변수
    while True:
        try:
            data = client_socket.recv(1024).decode('utf-8')  # 1024바이트의 문자열을 받음

            if not data:
                # 데이터가 비어 있으면 연결이 끊어졌으므로 반복문 종료
                break

            message += data

            while '\n' in message:
                # '\n'이 나타날 때마다 메시지를 처리하고, 남은 부분은 다음 반복에서 처리
                message, _, remaining = message.partition('\n')
                chat_display.insert(tk.END, message + '\n')
                process_message(message)
                message = remaining

        except ConnectionAbortedError:
            # 연결이 강제로 종료되면 반복문 종료
            break

#소켓 데이터 메시지에 따른 GUI출력
def process_message(message):
    global is_tester
    global is_host
    global is_muted
    global NOT_LAST_Client
    global first_client 
    
    #마지막클라이언트 아닌 클라이언트에게 받는 투표 출력 메시지
    if "NotLaSTCLientHERE" in message : #11
        NOT_LAST_Client = True
    elif "YOuiSLASTCliENT" in message : #3
        NOT_LAST_Client = False
        
    # 닉네임 생성완료 후 GUI출력 
    elif "NickNameSETTING..!" in message: #6
        chat_display.grid(row=0, column=0, padx=10, pady=10, columnspan=2)    
        send_button.grid(row=1, column=1, padx=10, pady=10)
        input_entry.grid(row=1, column=0, padx=10, pady=10)
        if first_client :
            ready_button.grid(row=3, column=0, columnspan=2, padx=10, pady=10)

    # 게임 시작 메시지를 확인하고 투표창 출력
    elif "Please Vote" in message: #2
        selected_game = show_vote_dialog()
        
    elif "YouIsFisrtClient" in message: #1
        first_client  = True
        chat_display.insert(tk.END, 'first_client = True\n')
    #준비완료버튼생성메시지 
    elif "Are you Ready?" in message :
        ready_button.grid(row=3, column=0, columnspan=2, padx=10, pady=10)
    
    #테스터 선정메시지
    elif "YOU IS TESTER. PLEASE ENTER THE ANSWER" in message: #7
        is_tester = True
        chat_display.insert(tk.END, 'is_tester = True\n')
        show_answer_input_gui()
        
    #호스트 선정메시지
    elif "YouIsHost!@!@.!@@!@" in message: #8
        is_host = True
        chat_display.insert(tk.END, 'is_host = True\n')
        show_question_input_gui()        

    # 발언권 차단 메시지
    elif "MuTeOnChangeByServer" in message: #4
        is_muted = True  # 음소거 상태로 설정 잠시 해제
        input_entry.config(state="disabled")
        send_button.config(state="disabled")
        
    # 발언권 차단 해제 메시지
    elif "MuTeOffChangeByServer" in message: #5
        is_muted = False  # 음소거 해제 상태로 설정
        input_entry.config(state="normal")
        send_button.config(state="normal")
        
    # YESNO GUI 생성 메시
    elif "YESNOYESNONOYESNOO" in message: #9
         show_YesNo_input_gui()

    #게임 끝 준비완료 생성 메시지
    elif "GameOffYouCanReady"in message: #12
        ready_button.grid(row=3, column=0, columnspan=2, padx=10, pady=10)
        chat_display.insert(tk.END, message + '\n')
        input_entry.config(state="normal")
        send_button.config(state="normal")

    elif "GameOVERrRRRrRRRRrR" in message:#10
        input_entry.config(state="normal")
        send_button.config(state="normal")
    #특정 메시지 아니면 모든 메시지 chat_display에 출력
    #else :
        #chat_display.insert(tk.END, message + '\n')



#채팅 데이터  전송 함수
def send_message():
    global nickname
    message = input_entry.get()
    if message :
        # 닉네임과 메시지를 조합해서 전송
        client_socket.send((nickname + ": " + message).encode('utf-8'))
        input_entry.delete(0, tk.END)

#닉네임 전송하면서 GUI 제거    
def set_nickname():
    global nickname
    nickname = nickname_entry.get()
    nickname_label.config(text=f"Nickname: {nickname}")
    nickname_label.grid_forget()  # 라벨 숨기기
    nickname_entry.grid_forget()  # 엔트리 숨기기
    set_nickname_button.grid_forget()  # 버튼 숨기기
    chat_display.insert(tk.END, f"닉네임 {nickname} 설정하였습니다.\n")
    input_entry.config(state="normal")  # 입력 창 활성화
    send_button.config(state="normal")  # Send 버튼 활성화
    client_socket.send(f"SET_NICKNAME {nickname}".encode('utf-8'))
    scroll_button.grid(row=4, column=0, padx=10, pady=5)

# 준비완료 전송 함수
def on_ready_button_click():

    global nickname
    message = f"SET_READY {nickname} is ready!"
    
    # 닉네임과 메시지를 조합해서 전송
    client_socket.send(message.encode('utf-8'))
    input_entry.delete(0, tk.END)
    #input_entry.config(state="disabled")
    #send_button.config(state="disabled")
    ready_button.grid_forget()

def scroll_to_bottom():
    chat_display.yview(tk.END)
    
def on_closing():
    client_socket.close()
    root.quit()

#투표 GUI
def show_vote_dialog():
    vote_window = tk.Toplevel(root)
    vote_window.title("게임 투표")

    game_options = ["스무고개", "문제풀이", "업다운", "에너지"]

    # 선택된 게임을 저장할 변수
    selected_game = tk.IntVar()

    # 라디오 버튼 생성 및 배치
    for i, option in enumerate(game_options):
        tk.Radiobutton(vote_window, text=option, variable=selected_game, value=i + 1).pack()

    # 확인 버튼 추가
    choice_button= tk.Button(vote_window, text="확인", command=lambda: on_vote_confirm(vote_window,selected_game))
    choice_button.pack()

    vote_window.wait_window()

    return selected_game.get()

#투표 전송 함수
def on_vote_confirm(vote_window, selected_game):
    # 선택된 게임을 서버에 전송
    client_socket.send(str(selected_game.get()).encode('utf-8'))
    if first_client or NOT_LAST_Client :
        client_socket.send(str(selected_game.get()).encode('utf-8'))
        
    vote_window.destroy()

#출제자 답 GUI
def show_answer_input_gui():
    global is_tester
    if is_tester:
        answer_window = tk.Toplevel(root)
        answer_window.title("Answer Input")
        
        # 라벨 및 엔트리 생성
        label = tk.Label(answer_window, text="You are the TESTER. Please enter your answer:")
        label.pack()

        entry = tk.Entry(answer_window)
        entry.pack()

        # 확인 버튼 추가
        button = tk.Button(answer_window, text="OK", command=lambda: on_answer_ok(answer_window, entry.get()))
        button.pack()

#출제자 답 전송 함수
def on_answer_ok(answer_window, answer):
    if answer:
        client_socket.send(f"{answer}".encode('utf-8'))
        if first_client or NOT_LAST_Client:
            client_socket.send(f"{answer}".encode('utf-8'))
        answer_window.destroy()  # 창 닫기


#제출자 질문 GUI
def show_question_input_gui():
    global is_host
    if is_host:
        question_window = tk.Toplevel(root)
        question_window.title("Question Input")
        
        # 라벨 및 엔트리 생성
        label = tk.Label(question_window, text="You are the HOST. Please enter your question:")
        label.pack()

        entry = tk.Entry(question_window)
        entry.pack()

        # 확인 버튼 추가
        button = tk.Button(question_window, text="OK", command=lambda: on_question_ok(question_window, entry.get()))
        button.pack()

#제출자 질문 전송  함수
def on_question_ok(question_window, question):
    if question:
        client_socket.send(f"{question}".encode('utf-8'))
        if first_client or NOT_LAST_Client :
            client_socket.send(f"{question}".encode('utf-8'))
        is_host = False
        question_window.destroy()


#출제자 YESNO GUI
def show_YesNo_input_gui():
    global is_tester
    if is_tester:
        YesNo_window = tk.Toplevel(root)
        YesNo_window.title("YesNo input")
        
        # yes 버튼 추가
        yes_button = tk.Button(YesNo_window, text="Yes", command=lambda:  on_yes_ok(YesNo_window))
        yes_button.pack()
        
        # no 버튼 추가
        no_button = tk.Button(YesNo_window, text="no", command=lambda: on_no_ok(YesNo_window))
        no_button.pack()

#출제자 YES 전송  함수
def on_yes_ok(YesNo_window):
        client_socket.send("Yesssssssssssssssseses".encode('utf-8'))
        if first_client or NOT_LAST_Client :
            client_socket.send("Yesssssssssssssssseses".encode('utf-8'))
        YesNo_window.destroy()

#출제자 NO 전송 함수
def on_no_ok(YesNo_window):
        client_socket.send("Nooooooooooooooooono".encode('utf-8'))
        if first_client or NOT_LAST_Client :
            client_socket.send("Nooooooooooooooooono".encode('utf-8'))
        YesNo_window.destroy()


root = tk.Tk()
root.title("채팅 클라이언트")

chat_display = scrolledtext.ScrolledText(root, wrap=tk.WORD, width=40, height=20)
scroll_button = tk.Button(root, text="맨 아래로 스크롤", command=scroll_to_bottom)

input_entry = tk.Entry(root, width=30)

send_button = tk.Button(root, text="보내기", command=send_message)


nickname_label = tk.Label(root, text="닉네임:")
nickname_label.grid(row=2, column=0, padx=10, pady=5)

nickname_entry = tk.Entry(root, width=20)
nickname_entry.grid(row=2, column=1, padx=5, pady=5)

set_nickname_button = tk.Button(root, text="닉네임 설정", command=set_nickname)
set_nickname_button.grid(row=2, column=2, padx=5, pady=5)

# 버튼을 생성하고 배치
ready_button = tk.Button(root, text="준비완료", command=on_ready_button_click)

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_address = ('127.0.0.1', 9000)
client_socket.connect(server_address)

receive_thread = threading.Thread(target=receive_messages)
receive_thread.start()

# 다이얼로그 결과를 저장할 변수
dialog_result = tk.IntVar()

root.protocol("WM_DELETE_WINDOW", on_closing)

root.mainloop()
