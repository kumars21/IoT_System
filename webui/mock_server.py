import flask

class DummyState:
    locked = False

app = flask.Flask(__name__)
bp = flask.Blueprint("api", __name__, url_prefix="/api")
state = DummyState()

@bp.after_request
def _add_cors_header(response):
    response.headers["Access-Control-Allow-Origin"] = "*"
    response.headers["Access-Control-Allow-Headers"] = "Content-Type"
    return response

@bp.post("/network/save")
def _network_save():
    return {"succeed": True}

@bp.post("/network/reset")
def _network_reset():
    return {"succeed": True}

@bp.get("/control/lock/state")
def _control_lock_state():
    return {"locked": state.locked}

@bp.post("/control/lock/set")
def _control_lock_toggle():
    data = flask.request.get_json()
    state.locked = data["locked"]
    return {"succeed": True}

if __name__ == "__main__":
    app.register_blueprint(bp)
    app.run(debug=True)
