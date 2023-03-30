from trilaterate import Position, FarAxisOrigin2DPositionCalculator


class AnchorSensor:
    

    def __init__(self, id: str, pos: Position):
        self._id = id
        self._name = "AnchorSensor_stud_name"
        self._pos = pos


    def is_equal(self, other_anchor):
        return self._id == other_anchor.get_id()


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




class TagSensor:
    

    _pos_calculator = FarAxisOrigin2DPositionCalculator()


    # classmethod can access class attribute but not instance
    @classmethod
    def get_position_calculator(self):
        return self._pos_calculator
    

    def __init__(self, id: str):
        self._id = id
        self._name = "TagSensor_stud_name"
        self._many_dist_to_anchor_and_anchor = []


    def get_name(self) -> str:
        return self._name
    

    def set_name(self, in_name: str):
        self._name = in_name


    def get_many_dist_to_anchor_and_anchor(self):
        return self._many_dist_to_anchor_and_anchor
    

    def set_dist_to_anchor_and_anchor(self, in_dist_to_anchor, in_anchor):
        is_dist_updated = False
        for i in range(len(self._many_dist_to_anchor_and_anchor)):
            _, anchor = self._many_dist_to_anchor_and_anchor[i]
            if anchor.is_equal(in_anchor):
                self._many_dist_to_anchor_and_anchor[i][0] = in_dist_to_anchor
                is_dist_updated = True
                break

        if not is_dist_updated:
            self._many_dist_to_anchor_and_anchor.append([in_dist_to_anchor, in_anchor])  
            

    def get_position(self):
        many_dist_to_anchor_and_anchor_pos = []
        for dist, anchor in self._many_dist_to_anchor_and_anchor:
            many_dist_to_anchor_and_anchor_pos.append( [dist, anchor.get_position()])
        
        return self.__class__._pos_calculator.get_position_or_null_position(many_dist_to_anchor_and_anchor_pos)