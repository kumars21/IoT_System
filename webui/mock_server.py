import flask

class User:
    def __init__(self, name, tag):
        self.name = name
        self.tag = tag

class SavedNetwork:
    ssid = "FakeSSID"
    password ="You don't know"

class TagGenerator:
    def __init__(self):
        self._index = 0

    def __call__(self):
        result = "tag_%03d" % (self._index,)
        self._index += 1
        return result

class DummyState:
    _new_tag = TagGenerator()

    locked = False
    users = []
    network = SavedNetwork()

    def add_user(self, name):
        user = User(name, self._new_tag())
        self.users.append(user)
        return user

    def delete_user(self, tag):
        for i, user in enumerate(self.users):
            if user.tag == tag:
                self.users.pop(i)
                return True
        return False

app = flask.Flask(__name__)
bp = flask.Blueprint("api", __name__, url_prefix="/api")
state = DummyState()
state.add_user("fish")
state.add_user("haha")
state.add_user("adamin")

@bp.after_request
def _add_cors_header(response):
    response.headers["Access-Control-Allow-Origin"] = "*"
    response.headers["Access-Control-Allow-Headers"] = "Content-Type"
    return response

@bp.get("/network/get")
def _network_get():
    return {
        "ssid": state.network.ssid,
        "password": state.network.password,
    }

@bp.post("/network/save")
def _network_save():
    data = flask.request.get_json()
    state.network.ssid = data["ssid"]
    state.network.password = data["password"]
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

def _to_user_item(user):
    return {"name": user.name, "tag": user.tag}

@bp.get("/users/get")
def _users_get():
    return {"users": tuple(map(_to_user_item, state.users))}

@bp.post("/users/add")
def _users_add():
    data = flask.request.get_json()
    user = state.add_user(data["name"])
    return {"succeed": True, "item": _to_user_item(user)}

@bp.post("/users/delete")
def _users_delete():
    data = flask.request.get_json()
    return {"succeed": state.delete_user(data["tag"])}

if __name__ == "__main__":
    app.register_blueprint(bp)
    app.run(debug=True)
