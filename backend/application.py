from os.path import join as pjoin
import subprocess

import flask
from flask import Flask, request, Response
from flask_sockets import Sockets

import logging
logging.basicConfig(level=logging.DEBUG)

db_dir = "./db"

application = Flask(__name__, static_folder="static", static_url_path='')
sockets = Sockets(application)

socket = None


@application.route("/modelxml", methods=['GET'])
def get_model_mesh_xml():
    return flask.send_from_directory('db', 'complex.xml')


@application.route("/model/sofa_seat.000.mesh.xml", methods=['GET'])
def get_model_sofa_mesh_xml():
    return flask.send_from_directory('db', 'sofa_seat.000.mesh.xml')


@application.route("/model/sofa_seat.000.mesh.mesh", methods=['GET'])
def get_model_sofa_mesh_mesh():
    return flask.send_from_directory('db', 'sofa_seat.000.mesh.mesh')


@application.route("/model", methods=['GET', 'POST'])
def model():
    if request.method == 'GET':
        return get_model()
    elif request.method == 'POST':
        return post_model()


def get_model():
    return flask.send_from_directory('db', 'complex.mesh')


def post_model():
    if not request.is_json:
        return {"message": "Not JSON"}, 400

    xml = request.get_json()['model']

    # TODO: validate XML?

    xml_file = pjoin(db_dir, "model.xml")
    mesh_file = pjoin(db_dir, "model.mesh")
    convert_log_filename = pjoin(db_dir, "convert.log")
    with open(xml_file, 'w') as f:
        f.write(xml)

    # TODO: stdout and stderr
    # TODO: check error code
    subprocess.call(['OgreXMLConverter',
                     '-E', 'big',  # Always an ARM device?
                     '-log',  convert_log_filename,
                     xml_file, mesh_file])

    # TODO: return output if debug
    return ""


@application.route("/model/meta", methods=['GET'])
def get_meta():
    with open(pjoin(db_dir, "meta-complex.json")) as f:
        return Response(f.read(), mimetype='application/json')


@application.route("/model/furniture", methods=['GET'])
def get_furniture():
    with open(pjoin(db_dir, "furniture.json")) as f:
        return Response(f.read(), mimetype='application/json')


@application.route("/model/meta", methods=['POST'])
def post_meta():
    if not request.is_json:
        return {"message": "Not JSON"}, 400

    print(request.get_data())

    with open(pjoin(db_dir, "meta.json"), 'wb') as f:
        f.write(request.get_data())

    global socket
    if socket:
        socket.send('update')

    return ""


@application.route('/')
def root():
    return application.send_static_file('index.html')


@sockets.route('/notifications')
def notifications(ws):
    global socket
    print("Got websocket connection")
    socket = ws

    socket.send('update')

    while not ws.closed:
        message = ws.receive()
        if message == None:
            continue

        socket = None
        ws.close(code=1002, message='')

        # print("Sending message:", message)
        # ws.send(message)


if __name__ == "__main__":
    from gevent import pywsgi
    from geventwebsocket.handler import WebSocketHandler
    server = pywsgi.WSGIServer(
        ('', 5000), application, handler_class=WebSocketHandler)
    server.serve_forever()
    #application.run(host = '0.0.0.0')
