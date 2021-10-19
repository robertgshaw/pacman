import time
from flask import Flask, render_template
from flask_socketio import SocketIO, send

app = Flask(__name__, static_folder='../build', static_url_path='/')
app.config['SECRET_KEY'] = 'mysecret'

socketIo = SocketIO(app, cors_allowed_origins="*")

app.debug = True
app.host = 'localhost'

@app.route('/api/time')
def get_current_time():
    return {'time': time.time()}

@socketIo.on('message')
def handleMessage(msg):
    print(msg)
    send(msg[::-1], broadcast=True)
    return None

if __name__ == '__main__':
    socketIo.run(app)