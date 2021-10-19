import time
from flask import Flask, render_template
from flask_sock import Sock

app = Flask(__name__, static_folder='../build', static_url_path='/')
sock = Sock(app)

@app.route('/')
def index():
    return app.send_static_file('index.html')

@app.route('/api/time')
def get_current_time():
    return {'time': time.time()}

@sock.route('/api/echo')
def echo(sock):
    while True:
        data = sock.receive()
        print(len(data))
        sock.send(data[::-1])

if __name__ == '__main__':
    app.run()



