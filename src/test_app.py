import unittest
import room_model

class TestApp(unittest.TestCase):
    

    def test_givenAnchorAndTagRange_whenAppRunning_thenPublishPosition(self):
        message_broker = room_model.MockMessageBroker()
        r = room_model.Room()
        tag_pos_publisher = room_model.TagPositionPublisher(message_broker)
        room_range_updater = room_model.RoomRangeUpdater(r, message_broker)
        r.add_observer(tag_pos_publisher)
        r.populate_with_default_anchor_and_tag()

        message_broker.on_message("{\"source\": \"2000\", \"destination\": \"1001\", \"range\": 0.5}")
        self.assertEqual(message_broker.get_recent_publish(), "{\"source\": \"2000\", \"x\": 0.500, \"y\": 0.500, \"z\": 0.000}")
        
        message_broker.on_message("{\"source\": \"2000\", \"destination\": \"1000\", \"range\": 19.94993734}")
        message_broker.on_message("{\"source\": \"2000\", \"destination\": \"1001\", \"range\": 19.88089535}")
        message_broker.on_message("{\"source\": \"2000\", \"destination\": \"1002\", \"range\": 19.7040605}")
        self.assertEqual(message_broker.get_recent_publish(), "{\"source\": \"2000\", \"x\": 3.000, \"y\": 10.000, \"z\": 0.000}")
        

        
        