import unittest
import math

import trilaterate
import room


class TestAnchor(unittest.TestCase):
    
    
    def setUp(self):
        self.anchor = room.Anchor("1000", trilaterate.Position(0, 22, 0))


    def test_canInitializeWithIdAndPosition(self):
        anchor_sensor = room.Anchor("1000", trilaterate.Position(0, 22, 0))


    def test_givenNewAnchor_thenNameIsEmpty(self):
        self.assertEqual(self.anchor.get_name(), "AnchorSensor_stud_name")
    

    def test_canSetNameAndGetName(self):
        self.anchor.set_name("myanchor")
        self.assertEqual("myanchor", self.anchor.get_name())


    def test_canSetAndGetPosition(self):
        self.anchor.set_position(trilaterate.Position(10, 20, 30))
        self.assertEqual(self.anchor.get_position(), trilaterate.Position(10, 20, 30))

    
    def test_canGetId(self):
        anchor = room.Anchor("1000", trilaterate.Position(10, 20, 30))
        self.assertEqual(anchor.get_id(), "1000")


    def test_givenTwoAnchorWithSameId_thenTwoAnchorIsEqual(self):
        anchor1 = room.Anchor("1000", trilaterate.Position(10, 20, 30))
        anchor2 = room.Anchor("1000", trilaterate.Position(40, 50, 60))

        self.assertTrue(anchor1.is_equal(anchor2))


    def test_givenTwoAnchorWithSameId_thenEqualityOperatorReturnTrue(self):
        anchor1 = room.Anchor("1000", trilaterate.Position(10, 20, 30))
        anchor2 = room.Anchor("1000", trilaterate.Position(40, 50, 60))

        self.assertTrue(anchor1 == anchor2)


class TestTag(unittest.TestCase):
    

    def setUp(self):
        self.tag = room.Tag("2000")


    def test_canInitializeWithId(self):
        tag_sensor = room.Tag("2000")
    

    def test_newTag_hasStudName(self):
        self.assertEqual(self.tag.get_name(), "TagSensor_stud_name")
    

    def test_canSetAndGetName(self):
        self.tag.set_name("mytag1")
        
        self.assertEqual(self.tag.get_name(), "mytag1")

    
    def test_givenNewTag_thenEmptyManyDistToAnchorAndAnchor(self):
        many_dist_to_anchor_and_anchor = self.tag.get_many_dist_to_anchor_and_anchor()
        
        self.assertEqual([], many_dist_to_anchor_and_anchor)


    def test_givenNewTag_whenUpsertDistToAnchorAndAnchor_thenGetDistToAnchorAndAnchor(self):
        dist_to_anchor = 2.43
        anchor = room.Anchor("1000", trilaterate.Position(10, 20, 30))

        self.tag.upsert_dist_to_anchor_and_anchor(dist_to_anchor, anchor)
        returned_dist_to_anchor, returned_anchor = self.tag.get_many_dist_to_anchor_and_anchor()[0]

        self.assertEqual(dist_to_anchor, returned_dist_to_anchor)
        self.assertTrue(anchor.is_equal(returned_anchor))


    def test_givenTagWithDistToAnchorAndAnchor_whenUpsertDistToAnchorAndAnchorWithDifferentDistToAnchorAndSameAnchor_thenGetUpdatedDistToAnchor(self):
        first_dist_to_anchor = 2.43
        anchor = room.Anchor("1000", trilaterate.Position(10, 20, 30))
        second_dist_to_anchor = 2.75

        self.tag.upsert_dist_to_anchor_and_anchor(first_dist_to_anchor, anchor)
        self.tag.upsert_dist_to_anchor_and_anchor(second_dist_to_anchor, anchor)
        returned_dist_to_anchor, returned_anchor = self.tag.get_many_dist_to_anchor_and_anchor()[0]

        self.assertEqual(second_dist_to_anchor, returned_dist_to_anchor)
        self.assertTrue(anchor.is_equal(returned_anchor))
        
        
    def test_givenTagWithManyDistToAnchorAndAnchor_whenGetPosition_thenReturnPosition(self):
        # Tag is assume at (3, 9, 17), but calculated pos will not have z pos
        exp_calc_tag_pos = trilaterate.Position(3, 9, 0)
        origin_anchor = room.Anchor("1000", trilaterate.Position(0, 0, 0))
        dist_to_origin = math.sqrt(379)
        far_x_anchor = room.Anchor("1001", trilaterate.Position(11, 0, 0))
        dist_to_far_x = math.sqrt(434)
        far_y_anchor = room.Anchor("1002", trilaterate.Position(0, 5, 0))
        dist_to_far_y = math.sqrt(314)
        
        self.tag.upsert_dist_to_anchor_and_anchor(dist_to_origin, origin_anchor)
        self.tag.upsert_dist_to_anchor_and_anchor(dist_to_far_x, far_x_anchor)
        self.tag.upsert_dist_to_anchor_and_anchor(dist_to_far_y, far_y_anchor)
        returned_tag_pos = self.tag.get_position()
        
        self.assertTrue(exp_calc_tag_pos.is_almost_equal(returned_tag_pos))


class TestRoom(unittest.TestCase):
    

    def setUp(self):
        self.room = room.Room()


    def test_canInitializeRoom(self):
        r = room.Room()


    def test_givenNewRoom_whenGetAnchor_thenReturnEmpty(self):
        many_anchor = self.room.get_many_anchor()
        self.assertEqual(many_anchor, [])


    def test_givenRoomWithAnchorIdAndAnchorPos_whenUpsertAnchor_thenReturnAnchor(self):
        origin_id = "1000"
        origin_pos = trilaterate.Position(0, 0, 0)
        far_x_id = "1001"
        far_x_pos = trilaterate.Position(21, 0, 0)
        origin = room.Anchor(origin_id, origin_pos)
        far_x = room.Anchor(far_x_id, far_x_pos)
        
        self.room.upsert_anchor_position(origin_id, origin_pos)
        self.room.upsert_anchor_position(far_x_id, far_x_pos)
        many_anchor = self.room.get_many_anchor()

        self.assertTrue(origin in many_anchor)
        self.assertTrue(far_x in many_anchor)        


    def test_givenRoomWithAnchorIdTenAndAnchorPos_whenUpsertAnchorIdTenAndNewAnchorPos_thenAnchorPosUpdatedToNewAnchorPos(self):
        anchor = room.Anchor("10", trilaterate.Position(40, 50, 60))

        self.room.upsert_anchor_position("10", trilaterate.Position(10, 20, 30))
        self.room.upsert_anchor_position("10", trilaterate.Position(40, 50, 60))
        self.room.upsert_anchor_position("22", trilaterate.Position(2, 2, 2))
        many_anchor = self.room.get_many_anchor()
        returned_anchor = [anchor for anchor in many_anchor if anchor.get_id() == "10"][0]

        self.assertEqual(anchor.get_position(), returned_anchor.get_position())


    def test_givenNewRoom_whenGetManyTag_thenManyTagIsEmpty(self):
        many_tag = self.room.get_many_tag()

        self.assertEqual(many_tag, [])


    def test_givenNewRoom_whenUpsertTagToAnchorDist_thenNothingHappen(self):
        tag_id = "2000"
        anchor_id = "1000"
        dist = 6.83

        self.room.upsert_tag_to_anchor_dist(tag_id, anchor_id, dist)
        many_tag = self.room.get_many_tag()

        self.assertEqual(many_tag, [])

        
    def test_givenRoomWithAnchor_whenUpsertTagToAnchorDist_thenTagIsUpdatedWithDistance(self):
        tag_id = "2000"
        anchor_id = "1000"
        dist = 6.83
        anchor_pos = trilaterate.Position(10, 20, 30)
        
        self.room.upsert_anchor_position(anchor_id, anchor_pos)
        self.room.upsert_tag_to_anchor_dist(tag_id, anchor_id, dist)
        many_tag = self.room.get_many_tag()
        returned_tag = many_tag[0]
        print(returned_tag)
        r_dist, anchor = returned_tag.get_many_dist_to_anchor_and_anchor()[0]
        
        self.assertEqual(r_dist, dist)
        self.assertEqual(anchor.get_id(), anchor_id)
        self.assertEqual(anchor.get_position(), anchor_pos)


    def test_givenNewRoom_whenGetManyObserver_thenManyObserverEmpty(self):
        many_observer = self.room.get_many_observer()

        self.assertEqual([], many_observer)


    def test_givenObserver_whenAddObserver_thenManyObserverHasObserver(self):
        tag_pos_publisher = room.TagPositionPublisher()

        self.room.add_observer(tag_pos_publisher)
        many_observer = self.room.get_many_observer()
        returned_observer = many_observer[0]

        self.assertTrue(tag_pos_publisher is returned_observer)


    def test_givenManyObserverHasObserver_whenNotifyObserver_thenObserverIsNotified(self):
        tag_pos_pub = room.TagPositionPublisher()
        self.room.add_observer(tag_pos_pub)
        
        initial_tag_pos_pub_notify_count = tag_pos_pub.get_notify_count()
        self.room.notify_observer()
        after_tag_pos_pub_notify_count = tag_pos_pub.get_notify_count()

        self.assertEqual(initial_tag_pos_pub_notify_count + 1, after_tag_pos_pub_notify_count)
        
        
    def test_givenManyObserverHasObserver_whenUpdateOrUpsertSuccessful_thenObserverIsNotified(self):
        mock_observer = room.MockRoomObserver()
        self.room.add_observer(mock_observer)
        
        initial_notify_count = mock_observer.get_notify_count()
        self.room.upsert_anchor_position("1000", trilaterate.Position(0, 0, 0))
        notify_count_A = mock_observer.get_notify_count()
        self.room.upsert_tag_to_anchor_dist("2000", "1000", 3.2)
        notify_count_B = mock_observer.get_notify_count()

        self.assertEqual(initial_notify_count + 1, notify_count_A)
        self.assertEqual(notify_count_A + 1, notify_count_B)
      

    def test_givenNewRoom_whenPopulateWithDefaultAnchorAndTag_thenRoomHasDefaultAnchorAndTag(self):
        origin = room.Anchor("1000", trilaterate.Position(0, 0, 0))
        far_x = room.Anchor("1001", trilaterate.Position(0.5, 0, 0))
        far_y = room.Anchor("1002", trilaterate.Position(0, 0.5, 0))
        tag = room.Tag("2000")
        tag_pos = trilaterate.Position(0.5, 0.5, 0)

        self.room.populate_with_default_anchor_and_tag()
        many_anchor = self.room.get_many_anchor()
        many_tag = self.room.get_many_tag()
        returned_tag = many_tag[0]
        returned_tag_pos = returned_tag.get_position()

        self.assertIn(origin, many_anchor)
        self.assertIn(far_x, many_anchor)
        self.assertIn(far_y, many_anchor)
        self.assertTrue(tag_pos.is_almost_equal(returned_tag_pos))

        

        
class TestAnchorCollection(unittest.TestCase):
    

    def setUp(self):
        self.anchor_collection = room.AnchorCollection()
        

    def test_givenInitializedAnchorCollection_thenEmptyCollection(self):
        many_anchor = self.anchor_collection.get_many_anchor()
        
        self.assertEqual(many_anchor, [])


    def test_givenAnchorIdAndPos_whenInsertAnchor_thenCollectionHasAnchor(self):
        anchor_id = "1000"
        anchor_pos = trilaterate.Position(10, 20, 30)

        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        many_anchor = self.anchor_collection.get_many_anchor()
        returned_anchor = many_anchor[0]

        self.assertEqual(anchor_id, returned_anchor.get_id())
        self.assertEqual(anchor_pos, returned_anchor.get_position())

    
    def test_givenAnchorIdAndPos_whenInsertAndCollectionHasSameExistingAnchorId_thenRaiseException(self):
        anchor_id = "1000"
        anchor_pos = trilaterate.Position(10, 20, 30)

        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)

        self.assertRaises(room.AnchorCollection.InsertSameAnchorIdException, self.anchor_collection.insert_anchor_id_and_pos, anchor_id, anchor_pos )
        self.assertRaises(room.AnchorCollection.InsertSameAnchorIdException, self.anchor_collection.insert_anchor_id_and_pos, anchor_id, trilaterate.Position(6, 6, 6))


    def test_givenAnchorIdAndPos_whenUpdateCollectionWithNoSameExistingAnchorId_thenRaiseException(self):
        anchor_id = "1000"
        anchor_pos = trilaterate.Position(10, 20, 30)
        
        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        
        self.assertRaises(room.AnchorCollection.UpdateNonexistentAnchorIdException, self.anchor_collection.update_anchor_position, "666", trilaterate.Position(10, 20, 30))


    def test_givenAnchorIdAndPos_whenUpdateCollectionWithExistingAnchorId_thenUpdateAnchor(self):
        anchor_id = "1000"
        anchor_pos = trilaterate.Position(10, 20, 30)
        updated_pos = trilaterate.Position(40, 50, 60)
        
        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        self.anchor_collection.update_anchor_position(anchor_id, updated_pos)
        returned_anchor = self.anchor_collection.get_anchor(anchor_id)

        self.assertEqual(returned_anchor.get_id(), anchor_id)
        self.assertEqual(returned_anchor.get_position(), updated_pos)


    def test_givenAnchorId_whenGetAnchorWithNonexistentAnchor_thenRaiseException(self):
        anchor_id = "1000"
        anchor_pos = trilaterate.Position(10, 20, 30)

        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        
        self.assertRaises(room.AnchorCollection.GetNonexistentAnchorIdException, self.anchor_collection.get_anchor, "666")


    def test_givenAnchorId_whenGetAnchorWithExistingAnchor_thenReturnedAnchor(self):
        anchor_id = "1000"
        anchor_pos = trilaterate.Position(10, 20, 30)
        
        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        returned_anchor = self.anchor_collection.get_anchor(anchor_id)
        
        self.assertEqual(returned_anchor.get_id(), anchor_id)
        self.assertEqual(returned_anchor.get_position(), anchor_pos)


class TestTagCollection(unittest.TestCase):

    
    def setUp(self):
        self.tag_collection = room.TagCollection()


    def test_givenNewTagCollection_whenGetManyTag_thenManyTagIsEmpty(self):
        many_tag = self.tag_collection.get_many_tag()
        
        self.assertEqual([], many_tag)


    def test_givenInsertTag_whenEmptyCollection_thenManyTagHasTag(self):
        tag_id = "1000"
        
        self.tag_collection.insert_tag_by_id("1000")
        many_tag = self.tag_collection.get_many_tag()
        returned_tag = many_tag[0]

        self.assertEqual(returned_tag.get_id(), tag_id)

    
    def test_givenInsertTagById_whenCollectionHasTagWithSameId_thenRaiseException(self):
        self.tag_collection.insert_tag_by_id("1000")
        self.tag_collection.insert_tag_by_id("1001")

        self.assertRaises(room.TagCollection.InsertSameTagIdException, self.tag_collection.insert_tag_by_id, "1000")


    def test_givenGetTag_whenCollectionHasNoTagWithSameId_thenRaiseException(self):
        self.tag_collection.insert_tag_by_id("1000")

        self.assertRaises(room.TagCollection.GetNonexistentTagIdException, self.tag_collection.get_tag, "666")


    def test_givenGetTag_whenCollectionHasTag_thenReturnTag(self):
        self.tag_collection.insert_tag_by_id("1000")
        self.tag_collection.insert_tag_by_id("1001")

        returned_tag = self.tag_collection.get_tag("1000")

        self.assertEqual(room.Tag("1000"), returned_tag)


class TestTagToJSONStringConverter(unittest.TestCase):
    
    
    def setUp(self):
        self.converter = room.TagToJSONStringConverter()


    def test_givenTagWithNullPosition_whenGetJSONfromTag_thenRaiseTagHasNullPositionException(self):
        tag = room.Tag("2000")
        
        self.assertRaises(room.TagToJSONStringConverter.UnableToGetTagPositionException, self.converter.get_JSON, tag)
        
        
    def test_givenTagWithValidPosition_whenGetJSONFromTag_thenReturnStringInJSONformat(self):
        origin_id = "1000"
        origin_pos = trilaterate.Position(0, 0, 0)
        origin = room.Anchor(origin_id, origin_pos)
        far_x_id = "1001"
        far_x_pos = trilaterate.Position(11, 0, 0)
        far_x = room.Anchor(far_x_id, far_x_pos)
        far_y_id = "1002"
        far_y_pos = trilaterate.Position(0, 5, 0)
        far_y = room.Anchor(far_y_id, far_y_pos)
        tag_id = "2000"
        tag = room.Tag(tag_id)
        tag_to_origin = math.sqrt(379)
        tag_to_far_x = math.sqrt(434)
        tag_to_far_y = math.sqrt(314)
        # Actual tag pos mathematically would be (3, 9, 17)
        # but for now algorithm is only for 2D space
        expected_tag_pos = trilaterate.Position(3, 9, 0)

        tag.upsert_dist_to_anchor_and_anchor(tag_to_origin, origin)
        tag.upsert_dist_to_anchor_and_anchor(tag_to_far_x, far_x)
        tag.upsert_dist_to_anchor_and_anchor(tag_to_far_y, far_y)
        json_string = self.converter.get_JSON(tag)
        tag_pos = tag.get_position()
        tag_x_pos = tag_pos.get_x()
        tag_y_pos = tag_pos.get_y()
        tag_z_pos = tag_pos.get_z()


        self.assertEqual(json_string, f"\"source\": \"2000\", \"x\": {tag_x_pos}, \"y\": {tag_y_pos}, \"z\": {tag_z_pos}")
        

class TestTagPositionPublisher(unittest.TestCase):
    

    def setUp(self):
        self.position_publisher = room.TagPositionPublisher()


    def test_canBeNotify(self):
        r = room.Room()
        self.position_publisher.notify(r)


    def test_givenRoom_whenNotify_thenNotifyCountIncremented(self):
        r = room.Room()
        
        initial_notify_count = self.position_publisher.get_notify_count()
        self.position_publisher.notify(r)
        after_notify_count = self.position_publisher.get_notify_count()

        self.assertEqual(initial_notify_count + 1, after_notify_count)
        

    def test_givenRoom_whenNotify_thenPublishTagPoistion(self):
        r = room.Room()
        r.populate_with_default_anchor_and_tag()

        tag_pos_pub = room.TagPositionPublisher(room.MockMessageBroker())
        tag_pos_pub.notify(r)
        string_publisher = tag_pos_pub.get_message_broker()
        published_string = string_publisher.get_recent_publish()
        
        expected_string = f"\"source\": \"2000\", \"x\": 0.5, \"y\": 0.5, \"z\": 0"
        self.assertEqual(published_string, expected_string)


    def test_givenRoomWithTagWithNullPosition_whenNotify_thenPublishNothing(self):
        r = room.Room()

        tag_pos_pub = room.TagPositionPublisher(room.MockMessageBroker())
        tag_pos_pub.notify(r)
        string_publisher = tag_pos_pub.get_message_broker()
        published_string = string_publisher.get_recent_publish()

        self.assertEqual("", published_string)

        
class TestMockMessageBroker(unittest.TestCase):


    def setUp(self):
        self.mock_msg_broker = room.MockMessageBroker()


    def test_givenPublishStringX_whenGetPublished_thenReturnStringX(self):
        self.mock_msg_broker.publish("hello world")
        returned_publish = self.mock_msg_broker.get_recent_publish()

        self.assertEqual("hello world", returned_publish)
    
    
    def test_givenOnMessageCallback_whenSetOnMessageCallback_thenGetOnMessageCallbackReturnCallback(self):
        def simple_callback(in_str):
            pass
        callback = simple_callback

        self.mock_msg_broker.set_on_message_callback(callback)
        returned_callback = self.mock_msg_broker.get_on_message_callback()

        self.assertIs(callback, returned_callback)
        

    def test_givenHaveOnMessageCallback_whenOnMessage_thenExecuteCallback(self):
        def simple_callback(in_str):
            simple_callback.is_called = True
        simple_callback.is_called = False

        self.mock_msg_broker.set_on_message_callback(simple_callback)
        self.mock_msg_broker.on_message("hello")

        self.assertTrue(simple_callback.is_called)
  
        
class TestRoomRangeUpdater(unittest.TestCase):


    def setUp(self):
        r = room.Room()
        r.populate_with_default_anchor_and_tag()
        self.range_updater = room.RoomRangeUpdater(r)
    

    def test_givenRoom_whenInitialize_thenHasRoom(self):
        r = room.Room()
        range_updater = room.RoomRangeUpdater(r)
        
        returned_room = range_updater.get_room()

        self.assertIs(r, returned_room)


    def test_givenHasRoom_whenUpdateRoom_thenRoomIsUpdated(self):
        message = "{\"source\": \"2000\", \"destination\": \"1000\", \"range\": \"69\"}"
        
        self.range_updater.update_room(message)
        r = self.range_updater.get_room()
        tag = r.get_many_tag()[0]
        many_dist_to_anchor_and_anchor = tag.get_many_dist_to_anchor_and_anchor()
        for dist_and_anchor in many_dist_to_anchor_and_anchor:
            dist, anchor = dist_and_anchor
            if dist == 69:
                break

        self.assertEqual(dist, 69)
        self.assertEqual(anchor.get_id(), "1000")
        
        
    def test_givenMessageBroker_whenSetMessageBroker_thenGetMessageBrokerReturnMessageBroker(self):
        mock_message_broker = room.MockMessageBroker()

        self.range_updater.set_message_broker(mock_message_broker)
        returned_message_broker = self.range_updater.get_message_broker()

        self.assertIs(mock_message_broker, returned_message_broker)


    def test_givenSetMessageBroker_whenMessageBrokerOnMessage_thenRoomIsUpdated(self):
        mock_message_broker = room.MockMessageBroker()
        r = self.range_updater.get_room()

        self.range_updater.set_message_broker(mock_message_broker)
        mock_message_broker.on_message("{\"source\": \"2000\", \"destination\": \"1000\", \"range\": \"420\"}")
        many_tag = r.get_many_tag()
        tag: room.Tag = many_tag[0]
        for dist_and_anchor in tag.get_many_dist_to_anchor_and_anchor():
            dist, anchor = dist_and_anchor
            if anchor.get_id() == "1000":
                break

        self.assertEqual(dist, 420)
        

class TestMockRoomObserver(unittest.TestCase):

    
    def setUp(self):
        self.mock_observer = room.MockRoomObserver()


    def test_givenNew_whenGetNotifyCount_thenNotifyCountIsZero(self):
        returned_notify_count = self.mock_observer.get_notify_count()

        self.assertEqual(returned_notify_count, 0)


    def test_givenRoom_whenNotify_thenNotifyCountIncremented(self):
        r = room.Room()

        self.mock_observer.notify(r)

        self.assertEqual(self.mock_observer.get_notify_count(), 1)
    
    


        



class TestGoldenBehaviour(unittest.TestCase):
    

    def test_givenSensorData_thenReturnedPosition(self):
        r = room.Room()
        origin_id = "1000"
        origin_pos = trilaterate.Position(0, 0, 0)
        far_x_id = "1001"
        far_x_pos = trilaterate.Position(11, 0, 0)
        far_y_id = "1002"
        far_y_pos = trilaterate.Position(0, 5, 0)
        tag_id = "2000"
        tag_name = "alice"
        tag_to_origin = math.sqrt(379)
        tag_to_far_x = math.sqrt(434)
        tag_to_far_y = math.sqrt(314)
        # Actual tag pos mathematically would be (3, 9, 17)
        # but for now algorithm is only for 2D space
        expected_tag_pos = trilaterate.Position(3, 9, 0)

        r.upsert_anchor_position(origin_id, origin_pos)
        r.upsert_anchor_position(far_x_id, far_x_pos)
        r.upsert_anchor_position(far_y_id, far_y_pos)
        r.upsert_tag_to_anchor_dist(tag_id, origin_id, tag_to_origin)
        r.upsert_tag_to_anchor_dist(tag_id, far_x_id, tag_to_far_x)
        r.upsert_tag_to_anchor_dist(tag_id, far_y_id, tag_to_far_y)
        many_tag = r.get_many_tag()
        tag = many_tag[0]
        calculated_tag_pos = tag.get_position()
        print(calculated_tag_pos)

        self.assertTrue(expected_tag_pos.is_almost_equal(calculated_tag_pos))

    