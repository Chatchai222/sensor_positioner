import math

class Position:
    
    _ROUND_TO_NEAREST_DECIMAL_DIGIT = 6


    def __init__(self, x: float, y: float, z: float):
        self._x = x
        self._y = y
        self._z = z

    
    def is_null(self) -> bool:
        return False
        

    def is_almost_equal(self, other_pos, error_dist_threshold=0.001) -> bool:
        dist_diff = self.get_dist_to(other_pos)
        return dist_diff < error_dist_threshold
            

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
        return round(self._x, self._ROUND_TO_NEAREST_DECIMAL_DIGIT)
    

    def get_y(self) -> float:
        return round(self._y, self._ROUND_TO_NEAREST_DECIMAL_DIGIT)
    

    def get_z(self) -> float:
        return round(self._z, self._ROUND_TO_NEAREST_DECIMAL_DIGIT)
    

    def __str__(self):
        return self.to_string()
    

    def __repr__(self):
        return self.to_string()
    

    def __eq__(self, other):
        return self._x == other.get_x() and self._y == other.get_y() and self._z == other.get_z()


class FarAxisOrigin2DPositionCalculator:
    """
    dist_to_anchor = [
        [3.24, Position(0, 0, 0)],
        [15.42, Position(12.5, 0, 0)],
        [9.12, Position(0, 7.7, 0)],
    ]
    """

    def __init__(self):
        pass

    
    def get_position(self, many_tag_dist_to_anchor_pos) -> Position:
        tag_dist_to_origin_pos, origin_pos = self._get_tag_dist_to_origin_and_origin_pos(many_tag_dist_to_anchor_pos)
        tag_dist_to_far_x_pos, far_x_pos = self._get_tag_dist_to_far_x_and_far_x_pos(many_tag_dist_to_anchor_pos)
        tag_dist_to_far_y_pos, far_y_pos = self._get_tag_dist_to_far_y_and_far_y_pos(many_tag_dist_to_anchor_pos)

        origin_dist_to_far_x = origin_pos.get_dist_to(far_x_pos)
        origin_dist_to_far_y = origin_pos.get_dist_to(far_y_pos)
        
        tag_x_coord = self._get_single_axis_coordinate(origin_dist_to_far_x, tag_dist_to_origin_pos, tag_dist_to_far_x_pos)
        tag_y_coord = self._get_single_axis_coordinate(origin_dist_to_far_y, tag_dist_to_origin_pos, tag_dist_to_far_y_pos)

        tag_pos = Position(tag_x_coord, tag_y_coord, 0)
        return tag_pos


    def _get_tag_dist_to_origin_and_origin_pos(self, many_tag_dist_and_anchor_pos):
        for tag_dist, anchor_pos in many_tag_dist_and_anchor_pos:
            if anchor_pos == Position(0, 0, 0):
                return tag_dist, anchor_pos
            
        raise self.__class__.MissingOriginPositionException
    

    def _get_tag_dist_to_far_x_and_far_x_pos(self, many_tag_dist_and_anchor_pos):
        for tag_dist, anchor_pos in many_tag_dist_and_anchor_pos:
            if anchor_pos.get_x() != 0 and anchor_pos.get_y() == 0 and anchor_pos.get_z() == 0:
                return tag_dist, anchor_pos
            
        raise self.__class__.MissingFarXPositionException
    

    def _get_tag_dist_to_far_y_and_far_y_pos(self, many_tag_dist_and_anchor_pos):
        for tag_dist, anchor_pos in many_tag_dist_and_anchor_pos:
            if anchor_pos.get_x() == 0 and anchor_pos.get_y() != 0 and anchor_pos.get_z() == 0:
                return tag_dist, anchor_pos
            
        raise self.__class__.MissingFarYPositionException


    def _get_single_axis_coordinate(self, origin_dist_to_far_axis, tag_dist_to_origin, tag_dist_to_far_axis) -> float:
        of = origin_dist_to_far_axis
        to = tag_dist_to_origin
        tf = tag_dist_to_far_axis

        coord = ( (tf * tf) - (to * to) - (of * of) ) / (-2 * of)
        return coord


    class MissingOriginPositionException(Exception):
        pass


    class MissingFarXPositionException(Exception):
        pass


    class MissingFarYPositionException(Exception):
        pass