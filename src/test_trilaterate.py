import unittest
import trilaterate
import math

class TestPosition(unittest.TestCase):
    

    def test_givenTwoPositionWithSmallDistance_thenPositionIsAlmostEqual(self):
        # This test is for floating point error 
        pos1 = trilaterate.Position(10, 20, 30)
        pos2 = trilaterate.Position(10.00001, 19.9999, 30.0003)
        pos3 = trilaterate.Position(9.99999, 20.00001, 29.99999)
        
        self.assertTrue(pos1.is_almost_equal(pos2))
        self.assertTrue(pos1.is_almost_equal(pos3))


    def test_givenTwoPositionWithSameCoordinate_thenEqualityOperatorReturnTrue(self):
        pos1 = trilaterate.Position(10, 20, 30)
        pos2 = trilaterate.Position(10, 20, 30)

        self.assertTrue(pos1 == pos2)


class TestFarAxisOrigin2DPositionCalculator(unittest.TestCase):

    
    def setUp(self):
        self.pos_calculator = trilaterate.FarAxisOrigin2DPositionCalculator()

    def test_givenEmptyList_whenGetPosition_thenRaiseMissingOriginPosition(self):
        self.assertRaises(trilaterate.FarAxisOrigin2DPositionCalculator.MissingOriginPositionException, self.pos_calculator.get_position, [])
        

    def test_givenMissingOriginPosition_whenGetPosition_thenRaiseMissingOriginPosition(self):
        many_tag_dist_to_anchor_pos = [ 
            (math.sqrt(66), trilaterate.Position(22, 0, 0)),
            (math.sqrt(67), trilaterate.Position(0, 22, 0)),
            (math.sqrt(342), trilaterate.Position(0, 0, 22)),
            (math.sqrt(22), trilaterate.Position(32, 12, 23)),
        ]

        self.assertRaises(self.pos_calculator.__class__.MissingOriginPositionException, self.pos_calculator.get_position, many_tag_dist_to_anchor_pos)


    def test_givenMissingFarXPosition_whenGetPosition_thenRaiseMissingFarXPosition(self):
        many_tag_dist_to_anchor_pos = [ 
            (math.sqrt(66), trilaterate.Position(0, 0, 0)),
            (math.sqrt(67), trilaterate.Position(0, 22, 0)),
            (math.sqrt(342), trilaterate.Position(0, 0, 22)),
            (math.sqrt(22), trilaterate.Position(32, 12, 23)),
        ]

        self.assertRaises(self.pos_calculator.__class__.MissingFarXPositionException, self.pos_calculator.get_position, many_tag_dist_to_anchor_pos)


    def test_givenMissingFarYPosition_whenGetPosition_thenRaiseMissingFarYPosition(self):
        many_tag_dist_to_anchor_pos = [ 
            (math.sqrt(66), trilaterate.Position(0, 0, 0)),
            (math.sqrt(67), trilaterate.Position(13, 0, 0)),
            (math.sqrt(342), trilaterate.Position(0, 0, 22)),
            (math.sqrt(22), trilaterate.Position(32, 12, 23)),
        ]

        self.assertRaises(self.pos_calculator.__class__.MissingFarYPositionException, self.pos_calculator.get_position, many_tag_dist_to_anchor_pos)


    def test_givenCompleteData_returnCorrectPosition(self):
        tag_dist_to_anchor_pos_and_anchor_pos = [
            (math.sqrt(318), trilaterate.Position(0, 0, 0)), # origin
            (math.sqrt(230), trilaterate.Position(22, 0, 0)), # far x
            (math.sqrt(267), trilaterate.Position(0, 3, 0)), # far y
        ]
        expected_tag_position = trilaterate.Position(13, 10, 0)

        calculated_tag_position = self.pos_calculator.get_position(tag_dist_to_anchor_pos_and_anchor_pos)

        self.assertEqual(expected_tag_position.get_x(), round(calculated_tag_position.get_x()))
        self.assertEqual(expected_tag_position.get_y(), round(calculated_tag_position.get_y()))
    

if __name__ == '__main__':
    unittest.main()