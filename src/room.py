from abc import ABC, abstractmethod
import numbers


class Position:
    def __init__(self,x: float, y: float, z: float):
        self._x = x
        self._y = y
        self._z = z

    
    def is_null(self):
        return False
    

    def to_string(self) -> str:
        return f"x:{self._x}, y: {self._y}, z: {self._z}"
    
    
    def set_x(self, x: float):
        self._x = x
    

    def set_y(self, y: float):
        self._y = y


    def set_z(self, z: float):
        self._z = z


    def get_x(self) -> float:
        return self._x
    

    def get_y(self) -> float:
        return self._y
    

    def get_z(self) -> float:
        return self._z
    

class NullPosition(Position):


    def __init__(self):
        pass


    def is_null(self):
        return True
    

    def to_string(self):
        return "null position"
    

    def set_x(self, x: float):
        raise 
    

    def set_y(self, y: float):
        self._y = y


    def set_z(self, z: float):
        self._z = z


    def get_x(self) -> float:
        return self._x
    

    def get_y(self) -> float:
        return self._y
    

    def get_z(self) -> float:
        return self._z


class PositionCalculator(ABC):


    @abstractmethod
    def get_position_or_none(self, many_dist_to_anchor) -> Position:
        raise NotImplementedError
    

class FarAxisOrigin2DPositionCalculator(PositionCalculator):
    """
    dist_to_anchor = [
        [3.24, Position(0, 0, 0)],
        [6.42, Position(12.5, 0, 0)],
        [9.12, Position(0, 7.7, 0)],
    ]
    """

    def __init__(self):
        pass

    
    def get_position_or_null_position(self, many_dist_to_anchor) -> Position:
        if self._is_position_calculatable(many_dist_to_anchor):
            return self._calculate_position(many_dist_to_anchor)
        else:
            return NullPosition()
        

    def _is_position_calculatable(self, many_dist_to_anchor) -> bool:
        if not self._is_many_dist_to_anchor_correct_format(many_dist_to_anchor):
            return False
        if not self._is_many_dist_to_anchor_has_orthogonal_anchor(many_dist_to_anchor):
            return False
        if not self._is_many_dist_to_anchor_has_origin_anchor(many_dist_to_anchor):
            return False
    
    def _is_many_dist_to_anchor_correct_format(self, many_dist_to_anchor) -> bool:
        pass


    def _calculate_position(self, many_dist_to_anchor) -> Position:
        pass
        
    

class TagSensor:
    def __init__(self, id):
        self.id = id
        self.s

class AnchorSensor:
    pass

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

    