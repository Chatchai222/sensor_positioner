import unittest
from trilaterate import Position
from room import AnchorSensor

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
        