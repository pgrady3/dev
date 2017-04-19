from flask import Flask
from flask import request
from DEVData import DEVData
from collections import deque
import json

app = Flask(__name__)
data_queue = deque()
last_session_name = ''


@app.route('/upload/', methods=['POST'])
def handle_post():
    global last_session_name
    try:
        data = DEVData(request.json)
    except ValueError:
        return 'The data format is not correct', 412
    write_to_file(data)
    if last_session_name != data.sessionName:
        data_queue.clear()
        last_session_name = data.sessionName
    data_queue.append(data)
    return 'Data accepted'


@app.route('/datarequest/', methods=['GET'])
def handle_get():
    if len(data_queue) > 0:
        result = json.dumps([i.__dict__ for i in data_queue])
        data_queue.clear()
        return result
    else:
        return 'No data available', 404


def write_to_file(data):
    f = open(data.sessionName + '.txt', 'a+')
    f.write(data.__str__())
    f.write('\n')
    f.close()


if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000, threaded=True)
