from trilaterate import Position


class AnchorSensor:
    

    def __init__(self, id: str, pos: Position):
        self._id = id
        self._name = "AnchorSensor_stud_name"
        self._pos = pos


    def get_id(self):
        return self._id


    def get_name(self):
        return self._name
    

    def get_position(self):
        return self._pos
    

    def set_name(self, in_name: str):
        self._name = in_name


    def set_position(self, in_pos: Position):
        self._pos = in_pos



    


    

    