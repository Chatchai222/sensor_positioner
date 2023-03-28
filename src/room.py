from abc import ABC, abstractmethod
import math

class Position:
    def __init__(self, x: float, y: float, z: float):
        self._x = x
        self._y = y
        self._z = z

    
    def is_null(self) -> bool:
        return False
    
    
    def is_equal(self, other_pos) -> bool:
        other_x = other_pos.get_x()
        other_y = other_pos.get_y()
        other_z = other_pos.get_z()
        
        return other_x == self._x and other_y == self._y and other_z == self._z
        

    def get_dist_to(self, other_pos) -> float:
        o_x = other_pos.get_x()
        o_y = other_pos.get_y()
        o_z = other_pos.get_z()
        
        return math.sqrt( (self._x - o_x) ** 2 + (self._y - o_y) ** 2 + (self._z - o_z) ** 2)

    
    def is_x_orthogonal(self, other_pos) -> float:
        o_y = other_pos.get_y()
        o_z = other_pos.get_z()
        
        return self._y == o_y and self._z == o_z
    
    
    def is_y_orthogonal(self, other_pos) -> float:
        o_x = other_pos.get_x()
        o_z = other_pos.get_z()

        return self._x == o_x and self._z == o_z
    

    def is_z_orthogonal(self, other_pos) -> float:
        o_x = other_pos.get_x()
        o_y = other_pos.get_y()
        
        return self._x == o_x and self._y == o_y


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
    

    def __str__(self):
        return self.to_string()
    

    def __repr__(self):
        return self.to_string()
    

class NullPosition(Position):


    def __init__(self):
        pass


    def is_null(self):
        return True
    

    def to_string(self):
        return "null position"
    

    def set_x(self, x: float):
        raise NotImplementedError
    

    def set_y(self, y: float):
        raise NotImplementedError


    def set_z(self, z: float):
        raise NotImplementedError


    def get_x(self) -> float:
        raise NotImplementedError
    

    def get_y(self) -> float:
        raise NotImplementedError
    

    def get_z(self) -> float:
        raise NotImplementedError


class PositionCalculator(ABC):


    @abstractmethod
    def get_position_or_null_position(self, many_tag_dist_to_anchor) -> Position:
        raise NotImplementedError
    

class FarAxisOrigin2DPositionCalculator(PositionCalculator):
    """
    dist_to_anchor = [
        [3.24, Position(0, 0, 0)],
        [15.42, Position(12.5, 0, 0)],
        [9.12, Position(0, 7.7, 0)],
    ]
    """

    def __init__(self):
        pass

    
    def get_position_or_null_position(self, many_tag_dist_to_anchor_pos) -> Position:
        try:
            return self._get_position(many_tag_dist_to_anchor_pos)
        except Exception as e:
            print(e)
            return NullPosition()
        

    # def _is_position_calculatable(self, many_tag_dist_to_anchor_pos) -> bool:
    #     if not self._is_many_tag_dist_to_anchor_pos_correct_format(many_tag_dist_to_anchor_pos):
    #         return False
    #     if not self._is_many_tag_dist_to_anchor_pos_has_origin_anchor_pos(many_tag_dist_to_anchor_pos):
    #         return False
    #     if not self._is_many_tag_dist_to_anchor_pos_has_far_x_anchor_pos(many_tag_dist_to_anchor_pos):
    #         return False
    #     if not self._is_many_tag_dist_to_anchor_pos_has_far_y_anchor_pos(many_tag_dist_to_anchor_pos):
    #         return False
        
    
    # def _is_many_dist_to_anchor_correct_format(self, many_dist_to_anchor_pos) -> bool:
    #     pass


    def _get_position(self, many_tag_dist_and_anchor_pos) -> Position:
        # tag_dist_to_origin_pos = self._get_tag_dist_to_origin_pos(many_tag_dist_to_anchor_pos)
        # tag_dist_to_far_x_pos = self._get_tag_dist_to_far_x_pos(many_tag_dist_to_anchor_pos)
        # tag_dist_to_far_y_pos = self._get_tag_dist_to_far_y_pos(many_tag_dist_to_anchor_pos)

        # origin_pos = self._get_origin_pos(many_tag_dist_to_anchor_pos)
        # far_x_pos = self._get_far_x_pos(many_tag_dist_to_anchor_pos)
        # far_y_pos = self._get_far_y_pos(many_tag_dist_to_anchor_pos)

        tag_dist_to_origin_pos, origin_pos = self._get_tag_dist_to_origin_and_origin_pos(many_tag_dist_and_anchor_pos)
        tag_dist_to_far_x_pos, far_x_pos = self._get_tag_dist_to_far_x_and_far_x_pos(many_tag_dist_and_anchor_pos)
        tag_dist_to_far_y_pos, far_y_pos = self._get_tag_dist_to_far_y_and_far_y_pos(many_tag_dist_and_anchor_pos)

        origin_dist_to_far_x = origin_pos.get_dist_to(far_x_pos)
        origin_dist_to_far_y = origin_pos.get_dist_to(far_y_pos)
        
        tag_x_coord = self._get_single_axis_coordinate(origin_dist_to_far_x, tag_dist_to_origin_pos, tag_dist_to_far_x_pos)
        tag_y_coord = self._get_single_axis_coordinate(origin_dist_to_far_y, tag_dist_to_origin_pos, tag_dist_to_far_y_pos)

        tag_pos = Position(tag_x_coord, tag_y_coord, 0)
        return tag_pos


    def _get_tag_dist_to_origin_and_origin_pos(self, many_tag_dist_and_anchor_pos):
        for tag_dist, anchor_pos in many_tag_dist_and_anchor_pos:
            if anchor_pos.is_equal(Position(0, 0, 0)):
                return tag_dist, anchor_pos
            
        raise Exception("Tag distance and anchor pos don't have origin anchor")
    

    def _get_tag_dist_to_far_x_and_far_x_pos(self, many_tag_dist_and_anchor_pos):
        origin = Position(0, 0, 0)
        for tag_dist, anchor_pos in many_tag_dist_and_anchor_pos:
            if origin.is_x_orthogonal(anchor_pos) and anchor_pos.get_x() != 0:
                return tag_dist, anchor_pos
            
        raise Exception("Tag distance and anchor pos don't have far x anchor")
    

    def _get_tag_dist_to_far_y_and_far_y_pos(self, many_tag_dist_and_anchor_pos):
        origin = Position(0, 0, 0)
        for tag_dist, anchor_pos in many_tag_dist_and_anchor_pos:
            if origin.is_y_orthogonal(anchor_pos) and anchor_pos.get_y() != 0:
                return tag_dist, anchor_pos
            
        raise Exception("Tag distance and anchor pos don't have far y anchor")


    def _get_single_axis_coordinate(self, origin_dist_to_far_axis, tag_dist_to_origin, tag_dist_to_far_axis) -> float:
        of = origin_dist_to_far_axis
        to = tag_dist_to_origin
        tf = tag_dist_to_far_axis

        coord = ( (tf * tf) - (to * to) - (of * of) ) / (-2 * of)
        return coord


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

    