from abc import ABC, abstractmethod
from trilaterate import Position, NullPosition, FarAxisOrigin2DPositionCalculator


class TagSensor:
    

    _position_calculator = FarAxisOrigin2DPositionCalculator() 


    def __init__(self, id):
        self._id = id
        self._name = "stud tagsensor name"
        self._many_dist_to_anchor_and_anchor = []

    
    def get_position(self) -> Position:
        many_dist_to_anchor_pos_and_anchor_pos = self._get_many_dist_to_anchor_and_anchor_pos()
        return TagSensor._position_calculator.get_position_or_null_position(many_dist_to_anchor_pos_and_anchor_pos)


    def get_id(self):
        return self._id
    

    def get_name(self):
        return self._name
    
    
    def set_name(self, in_name):
        self._name = in_name


    def set_dist_to_anchor(self, anchor, in_dist):
        pass                


    def _get_many_dist_to_anchor_and_anchor_pos(self):
        output = []
        for dist_to_anchor, anchor in self._many_dist_to_anchor_and_anchor:
            output.append((dist_to_anchor, anchor.get_position()))    
        
        return output
        

class AnchorSensor:
    

    def __init__(self, id, name="stud anchorsensor name", position=NullPosition()):
        self._id = id
        self._name =  name
        self._position = position


    def get_id(self):
        return self._id
    

    def get_name(self):
        return self._name
    

    def get_position(self):
        return self._position
    

    def set_name(self, in_name):
        self._name = in_name

    
    def set_position(self, in_position):
        self._position = in_position

            
        
class Room:

    def __init__(self):
        self.many_anchor = []
        self.many_tag = []
        pass


    def update_tag_to_anchor_distance(self, tag_id, anchor_id, distance):
        
        pass


    def update_anchor_position(self, anchor_id, position):
        pass


    def update_tag_name(self, name):
        pass

    
    def update_anchor_name(self, name):
        pass

    
    def get_many_tag_point(self):
        pass

    
    def get_many_anchor_point(self):
        pass

    