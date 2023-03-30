import unittest
import math

from trilaterate import Position, PositionCalculator
from room import AnchorSensor, TagSensor

class TestAnchorSensor(unittest.TestCase):
    
    
    def setUp(self):
        self.anchor = AnchorSensor("1000", Position(0, 22, 0))


    def test_canInitializeWithIdAndPosition(self):
        anchor_sensor = AnchorSensor("1000", Position(0, 22, 0))


    def test_givenNewAnchor_thenNameIsEmpty(self):
        self.assertEqual(self.anchor.get_name(), "AnchorSensor_stud_name")
    

    def test_canSetNameAndGetName(self):
        self.anchor.set_name("myanchor")
        self.assertEqual("myanchor", self.anchor.get_name())


    def test_canSetAndGetPosition(self):
        self.anchor.set_position(Position(10, 20, 30))
        self.assertTrue(self.anchor.get_position().is_equal(Position(10, 20, 30)))

    
    def test_canGetId(self):
        anchor = AnchorSensor("1000", Position(10, 20, 30))
        self.assertEqual(anchor.get_id(), "1000")


    def test_givenTwoAnchorWithSameId_thenTwoAnchorIsEqual(self):
        anchor1 = AnchorSensor("1000", Position(10, 20, 30))
        anchor2 = AnchorSensor("1000", Position(40, 50, 60))

        self.assertTrue(anchor1.is_equal(anchor2))


class TestTagSensor(unittest.TestCase):
    

    def setUp(self):
        self.tag = TagSensor("2000")


    def test_canInitializeWithId(self):
        tag_sensor = TagSensor("2000")
    

    def test_newTag_hasStudName(self):
        self.assertEqual(self.tag.get_name(), "TagSensor_stud_name")
    

    def test_canSetAndGetName(self):
        self.tag.set_name("mytag1")
        
        self.assertEqual(self.tag.get_name(), "mytag1")


    def test_classCanGetPositionCalculator(self):
        pos_calculator = TagSensor.get_position_calculator()
        pos_calculator_type = type(pos_calculator)
        
        self.assertTrue(issubclass(pos_calculator_type, PositionCalculator))

    
    def test_givenNewTag_thenEmptyManyDistToAnchorAndAnchor(self):
        many_dist_to_anchor_and_anchor = self.tag.get_many_dist_to_anchor_and_anchor()
        
        self.assertEqual([], many_dist_to_anchor_and_anchor)


    def test_givenNewTag_whenSetDistToAnchorAndAnchor_thenGetDistToAnchorAndAnchor(self):
        dist_to_anchor = 2.43
        anchor = AnchorSensor("1000", Position(10, 20, 30))

        self.tag.set_dist_to_anchor_and_anchor(dist_to_anchor, anchor)
        returned_dist_to_anchor, returned_anchor = self.tag.get_many_dist_to_anchor_and_anchor()[0]

        self.assertEqual(dist_to_anchor, returned_dist_to_anchor)
        self.assertTrue(anchor.is_equal(returned_anchor))


    def test_givenTagWithDistToAnchorAndAnchor_whenSetDistToAnchorAndAnchorWithDifferentDistToAnchorAndSameAnchor_thenGetUpdatedDistToAnchor(self):
        first_dist_to_anchor = 2.43
        anchor = AnchorSensor("1000", Position(10, 20, 30))
        second_dist_to_anchor = 2.75

        self.tag.set_dist_to_anchor_and_anchor(first_dist_to_anchor, anchor)
        self.tag.set_dist_to_anchor_and_anchor(second_dist_to_anchor, anchor)
        returned_dist_to_anchor, returned_anchor = self.tag.get_many_dist_to_anchor_and_anchor()[0]

        self.assertEqual(second_dist_to_anchor, returned_dist_to_anchor)
        self.assertTrue(anchor.is_equal(returned_anchor))
        
        
    def test_givenTagWithManyDistToAnchorAndAnchor_whenGetPosition_thenReturnPosition(self):
        # Tag is assume at (3, 9, 17), but calculated pos will not have z pos
        exp_calc_tag_pos = Position(3, 9, 0)
        origin_anchor = AnchorSensor("1000", Position(0, 0, 0))
        dist_to_origin = math.sqrt(379)
        far_x_anchor = AnchorSensor("1001", Position(11, 0, 0))
        dist_to_far_x = math.sqrt(434)
        far_y_anchor = AnchorSensor("1002", Position(0, 5, 0))
        dist_to_far_y = math.sqrt(314)
        
        self.tag.set_dist_to_anchor_and_anchor(dist_to_origin, origin_anchor)
        self.tag.set_dist_to_anchor_and_anchor(dist_to_far_x, far_x_anchor)
        self.tag.set_dist_to_anchor_and_anchor(dist_to_far_y, far_y_anchor)
        returned_tag_pos = self.tag.get_position()
        
        self.assertTrue(exp_calc_tag_pos.is_almost_equal(returned_tag_pos))