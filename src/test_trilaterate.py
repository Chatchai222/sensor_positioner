import unittest
from trilaterate import *
import math

class TestFarAxisOrigin2DPositionCalculator(unittest.TestCase):

    
    def test_givenEmptyList_returnNullPosition(self):
        pos_calculator = FarAxisOrigin2DPositionCalculator()
        result = pos_calculator.get_position_or_null_position([])
        self.assertTrue(result.is_null())


    def test_givenCompleteData_returnCorrectPosition(self):
        pos_calculator = FarAxisOrigin2DPositionCalculator()
        tag_dist_to_anchor_pos_and_anchor_pos = [
            (math.sqrt(318), Position(0, 0, 0)), # origin
            (math.sqrt(230), Position(22, 0, 0)), # far x
            (math.sqrt(267), Position(0, 3, 0)), # far y
        ]
        expected_tag_position = Position(13, 10, 0)

        calculated_tag_position = pos_calculator.get_position_or_null_position(tag_dist_to_anchor_pos_and_anchor_pos)

        self.assertEqual(expected_tag_position.get_x(), round(calculated_tag_position.get_x()))
        self.assertEqual(expected_tag_position.get_y(), round(calculated_tag_position.get_y()))
    

if __name__ == '__main__':
    unittest.main()