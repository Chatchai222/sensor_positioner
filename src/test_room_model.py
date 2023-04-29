import unittest
import math
import abc

import trilaterate_algorithm
import room_model


class TestAnchor(unittest.TestCase):
    
    
    def setUp(self):
        self.anchor = room_model.Anchor("1000", trilaterate_algorithm.Position(0, 22, 0))


    def test_canInitializeWithIdAndPosition(self):
        anchor_sensor = room_model.Anchor("1000", trilaterate_algorithm.Position(0, 22, 0))


    def test_givenNewAnchor_thenNameIsEmpty(self):
        self.assertEqual(self.anchor.get_name(), "AnchorSensor_stud_name")
    

    def test_canSetNameAndGetName(self):
        self.anchor.set_name("myanchor")
        self.assertEqual("myanchor", self.anchor.get_name())


    def test_canSetAndGetPosition(self):
        self.anchor.set_position(trilaterate_algorithm.Position(10, 20, 30))
        self.assertEqual(self.anchor.get_position(), trilaterate_algorithm.Position(10, 20, 30))

    
    def test_canGetId(self):
        anchor = room_model.Anchor("1000", trilaterate_algorithm.Position(10, 20, 30))
        self.assertEqual(anchor.get_id(), "1000")


    def test_givenTwoAnchorWithSameId_thenTwoAnchorIsEqual(self):
        anchor1 = room_model.Anchor("1000", trilaterate_algorithm.Position(10, 20, 30))
        anchor2 = room_model.Anchor("1000", trilaterate_algorithm.Position(40, 50, 60))

        self.assertTrue(anchor1.is_equal(anchor2))


    def test_givenTwoAnchorWithSameId_thenEqualityOperatorReturnTrue(self):
        anchor1 = room_model.Anchor("1000", trilaterate_algorithm.Position(10, 20, 30))
        anchor2 = room_model.Anchor("1000", trilaterate_algorithm.Position(40, 50, 60))

        self.assertTrue(anchor1 == anchor2)


class TestTag(unittest.TestCase):
    

    def setUp(self):
        self.tag = room_model.Tag("2000")


    def test_canInitializeWithId(self):
        tag_sensor = room_model.Tag("2000")
    

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
        anchor = room_model.Anchor("1000", trilaterate_algorithm.Position(10, 20, 30))

        self.tag.upsert_dist_to_anchor_and_anchor(dist_to_anchor, anchor)
        returned_dist_to_anchor, returned_anchor = self.tag.get_many_dist_to_anchor_and_anchor()[0]

        self.assertEqual(dist_to_anchor, returned_dist_to_anchor)
        self.assertTrue(anchor.is_equal(returned_anchor))


    def test_givenTagWithDistToAnchorAndAnchor_whenUpsertDistToAnchorAndAnchorWithDifferentDistToAnchorAndSameAnchor_thenGetUpdatedDistToAnchor(self):
        first_dist_to_anchor = 2.43
        anchor = room_model.Anchor("1000", trilaterate_algorithm.Position(10, 20, 30))
        second_dist_to_anchor = 2.75

        self.tag.upsert_dist_to_anchor_and_anchor(first_dist_to_anchor, anchor)
        self.tag.upsert_dist_to_anchor_and_anchor(second_dist_to_anchor, anchor)
        returned_dist_to_anchor, returned_anchor = self.tag.get_many_dist_to_anchor_and_anchor()[0]

        self.assertEqual(second_dist_to_anchor, returned_dist_to_anchor)
        self.assertTrue(anchor.is_equal(returned_anchor))
        
        
    def test_givenTagWithManyDistToAnchorAndAnchor_whenGetPosition_thenReturnPosition(self):
        # Tag is assume at (3, 9, 17), but calculated pos will not have z pos
        exp_calc_tag_pos = trilaterate_algorithm.Position(3, 9, 0)
        origin_anchor = room_model.Anchor("1000", trilaterate_algorithm.Position(0, 0, 0))
        dist_to_origin = math.sqrt(379)
        far_x_anchor = room_model.Anchor("1001", trilaterate_algorithm.Position(11, 0, 0))
        dist_to_far_x = math.sqrt(434)
        far_y_anchor = room_model.Anchor("1002", trilaterate_algorithm.Position(0, 5, 0))
        dist_to_far_y = math.sqrt(314)
        
        self.tag.upsert_dist_to_anchor_and_anchor(dist_to_origin, origin_anchor)
        self.tag.upsert_dist_to_anchor_and_anchor(dist_to_far_x, far_x_anchor)
        self.tag.upsert_dist_to_anchor_and_anchor(dist_to_far_y, far_y_anchor)
        returned_tag_pos = self.tag.get_position()
        
        self.assertTrue(exp_calc_tag_pos.is_almost_equal(returned_tag_pos))


class TestRoom(unittest.TestCase):
    

    def setUp(self):
        self.room = room_model.Room()


    def test_canInitializeRoom(self):
        r = room_model.Room()


    def test_givenNewRoom_whenGetAnchor_thenReturnEmpty(self):
        many_anchor = self.room.get_many_anchor()
        self.assertEqual(many_anchor, [])


    def test_givenRoomWithAnchorIdAndAnchorPos_whenUpsertAnchor_thenReturnAnchor(self):
        origin_id = "1000"
        origin_pos = trilaterate_algorithm.Position(0, 0, 0)
        far_x_id = "1001"
        far_x_pos = trilaterate_algorithm.Position(21, 0, 0)
        origin = room_model.Anchor(origin_id, origin_pos)
        far_x = room_model.Anchor(far_x_id, far_x_pos)
        
        self.room.upsert_anchor_position(origin_id, origin_pos)
        self.room.upsert_anchor_position(far_x_id, far_x_pos)
        many_anchor = self.room.get_many_anchor()

        self.assertTrue(origin in many_anchor)
        self.assertTrue(far_x in many_anchor)        


    def test_givenRoomWithAnchorIdTenAndAnchorPos_whenUpsertAnchorIdTenAndNewAnchorPos_thenAnchorPosUpdatedToNewAnchorPos(self):
        anchor = room_model.Anchor("10", trilaterate_algorithm.Position(40, 50, 60))

        self.room.upsert_anchor_position("10", trilaterate_algorithm.Position(10, 20, 30))
        self.room.upsert_anchor_position("10", trilaterate_algorithm.Position(40, 50, 60))
        self.room.upsert_anchor_position("22", trilaterate_algorithm.Position(2, 2, 2))
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
        anchor_pos = trilaterate_algorithm.Position(10, 20, 30)
        
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
        tag_pos_publisher = room_model.TagPositionPublisher()

        self.room.add_observer(tag_pos_publisher)
        many_observer = self.room.get_many_observer()
        returned_observer = many_observer[0]

        self.assertTrue(tag_pos_publisher is returned_observer)


    def test_givenManyObserverHasObserver_whenNotifyObserver_thenObserverIsNotified(self):
        tag_pos_pub = room_model.TagPositionPublisher()
        self.room.add_observer(tag_pos_pub)
        
        initial_tag_pos_pub_notify_count = tag_pos_pub.get_notify_count()
        self.room.notify_observer()
        after_tag_pos_pub_notify_count = tag_pos_pub.get_notify_count()

        self.assertEqual(initial_tag_pos_pub_notify_count + 1, after_tag_pos_pub_notify_count)
        
        
    def test_givenManyObserverHasObserver_whenUpdateOrUpsertSuccessful_thenObserverIsNotified(self):
        mock_observer = room_model.MockRoomObserver()
        self.room.add_observer(mock_observer)
        
        initial_notify_count = mock_observer.get_notify_count()
        self.room.upsert_anchor_position("1000", trilaterate_algorithm.Position(0, 0, 0))
        notify_count_A = mock_observer.get_notify_count()
        self.room.upsert_tag_to_anchor_dist("2000", "1000", 3.2)
        notify_count_B = mock_observer.get_notify_count()

        self.assertEqual(initial_notify_count + 1, notify_count_A)
        self.assertEqual(notify_count_A + 1, notify_count_B)
      

    def test_givenNewRoom_whenPopulateWithDefaultAnchorAndTag_thenRoomHasDefaultAnchorAndTag(self):
        origin = room_model.Anchor("1000", trilaterate_algorithm.Position(0, 0, 0))
        far_x = room_model.Anchor("1001", trilaterate_algorithm.Position(0.5, 0, 0))
        far_y = room_model.Anchor("1002", trilaterate_algorithm.Position(0, 0.5, 0))
        tag = room_model.Tag("2000")
        tag_pos = trilaterate_algorithm.Position(0.5, 0.5, 0)

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
        self.anchor_collection = room_model.AnchorCollection()
        

    def test_givenInitializedAnchorCollection_thenEmptyCollection(self):
        many_anchor = self.anchor_collection.get_many_anchor()
        
        self.assertEqual(many_anchor, [])


    def test_givenAnchorIdAndPos_whenInsertAnchor_thenCollectionHasAnchor(self):
        anchor_id = "1000"
        anchor_pos = trilaterate_algorithm.Position(10, 20, 30)

        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        many_anchor = self.anchor_collection.get_many_anchor()
        returned_anchor = many_anchor[0]

        self.assertEqual(anchor_id, returned_anchor.get_id())
        self.assertEqual(anchor_pos, returned_anchor.get_position())

    
    def test_givenAnchorIdAndPos_whenInsertAndCollectionHasSameExistingAnchorId_thenRaiseException(self):
        anchor_id = "1000"
        anchor_pos = trilaterate_algorithm.Position(10, 20, 30)

        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)

        self.assertRaises(room_model.AnchorCollection.InsertSameAnchorIdException, self.anchor_collection.insert_anchor_id_and_pos, anchor_id, anchor_pos )
        self.assertRaises(room_model.AnchorCollection.InsertSameAnchorIdException, self.anchor_collection.insert_anchor_id_and_pos, anchor_id, trilaterate_algorithm.Position(6, 6, 6))


    def test_givenAnchorIdAndPos_whenUpdateCollectionWithNoSameExistingAnchorId_thenRaiseException(self):
        anchor_id = "1000"
        anchor_pos = trilaterate_algorithm.Position(10, 20, 30)
        
        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        
        self.assertRaises(room_model.AnchorCollection.UpdateNonexistentAnchorIdException, self.anchor_collection.update_anchor_position, "666", trilaterate_algorithm.Position(10, 20, 30))


    def test_givenAnchorIdAndPos_whenUpdateCollectionWithExistingAnchorId_thenUpdateAnchor(self):
        anchor_id = "1000"
        anchor_pos = trilaterate_algorithm.Position(10, 20, 30)
        updated_pos = trilaterate_algorithm.Position(40, 50, 60)
        
        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        self.anchor_collection.update_anchor_position(anchor_id, updated_pos)
        returned_anchor = self.anchor_collection.get_anchor(anchor_id)

        self.assertEqual(returned_anchor.get_id(), anchor_id)
        self.assertEqual(returned_anchor.get_position(), updated_pos)


    def test_givenAnchorId_whenGetAnchorWithNonexistentAnchor_thenRaiseException(self):
        anchor_id = "1000"
        anchor_pos = trilaterate_algorithm.Position(10, 20, 30)

        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        
        self.assertRaises(room_model.AnchorCollection.GetNonexistentAnchorIdException, self.anchor_collection.get_anchor, "666")


    def test_givenAnchorId_whenGetAnchorWithExistingAnchor_thenReturnedAnchor(self):
        anchor_id = "1000"
        anchor_pos = trilaterate_algorithm.Position(10, 20, 30)
        
        self.anchor_collection.insert_anchor_id_and_pos(anchor_id, anchor_pos)
        returned_anchor = self.anchor_collection.get_anchor(anchor_id)
        
        self.assertEqual(returned_anchor.get_id(), anchor_id)
        self.assertEqual(returned_anchor.get_position(), anchor_pos)


class TestTagCollection(unittest.TestCase):

    
    def setUp(self):
        self.tag_collection = room_model.TagCollection()


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

        self.assertRaises(room_model.TagCollection.InsertSameTagIdException, self.tag_collection.insert_tag_by_id, "1000")


    def test_givenGetTag_whenCollectionHasNoTagWithSameId_thenRaiseException(self):
        self.tag_collection.insert_tag_by_id("1000")

        self.assertRaises(room_model.TagCollection.GetNonexistentTagIdException, self.tag_collection.get_tag, "666")


    def test_givenGetTag_whenCollectionHasTag_thenReturnTag(self):
        self.tag_collection.insert_tag_by_id("1000")
        self.tag_collection.insert_tag_by_id("1001")

        returned_tag = self.tag_collection.get_tag("1000")

        self.assertEqual(room_model.Tag("1000"), returned_tag)


class TestTagToJSONStringConverter(unittest.TestCase):
    
    
    def setUp(self):
        self.converter = room_model.TagToJSONStringConverter()


    def test_givenTagWithNullPosition_whenGetJSONfromTag_thenRaiseTagHasNullPositionException(self):
        tag = room_model.Tag("2000")
        
        self.assertRaises(room_model.TagToJSONStringConverter.UnableToGetTagPositionException, self.converter.get_JSON, tag)
        
        
    def test_givenTagWithValidPosition_whenGetJSONFromTag_thenReturnStringInJSONformat(self):
        origin_id = "1000"
        origin_pos = trilaterate_algorithm.Position(0, 0, 0)
        origin = room_model.Anchor(origin_id, origin_pos)
        far_x_id = "1001"
        far_x_pos = trilaterate_algorithm.Position(11, 0, 0)
        far_x = room_model.Anchor(far_x_id, far_x_pos)
        far_y_id = "1002"
        far_y_pos = trilaterate_algorithm.Position(0, 5, 0)
        far_y = room_model.Anchor(far_y_id, far_y_pos)
        tag_id = "2000"
        tag = room_model.Tag(tag_id)
        tag_to_origin = math.sqrt(379)
        tag_to_far_x = math.sqrt(434)
        tag_to_far_y = math.sqrt(314)
        # Actual tag pos mathematically would be (3, 9, 17)
        # but for now algorithm is only for 2D space
        expected_tag_pos = trilaterate_algorithm.Position(3, 9, 0)

        tag.upsert_dist_to_anchor_and_anchor(tag_to_origin, origin)
        tag.upsert_dist_to_anchor_and_anchor(tag_to_far_x, far_x)
        tag.upsert_dist_to_anchor_and_anchor(tag_to_far_y, far_y)
        json_string = self.converter.get_JSON(tag)
        tag_pos = tag.get_position()
        tag_x_pos = tag_pos.get_x()
        tag_y_pos = tag_pos.get_y()
        tag_z_pos = tag_pos.get_z()


        self.assertEqual(json_string, f"{{\"source\": \"2000\", \"x\": {tag_x_pos:.3f}, \"y\": {tag_y_pos:.3f}, \"z\": {tag_z_pos:.3f}}}")


class TestRoomObserver(unittest.TestCase):


    def test_roomObserver_isAbstractClass(self):
        self.assertTrue(issubclass(room_model.RoomObserver, abc.ABC))


    def test_roomObserver_canBeNotifiedOfRoom(self):
        notify_method = getattr(room_model.RoomObserver, 'notify')
        self.assertTrue(callable(notify_method))


class TestMockRoomObserver(unittest.TestCase):

    
    def setUp(self):
        self.mock_observer = room_model.MockRoomObserver()

    
    def test_givenNewInstance_whenCheckSubclass_thenIsSubclassRoomObserver(self):
        self.assertTrue(issubclass(type(self.mock_observer), room_model.RoomObserver))


    def test_givenNew_whenGetNotifyCount_thenNotifyCountIsZero(self):
        returned_notify_count = self.mock_observer.get_notify_count()

        self.assertEqual(returned_notify_count, 0)


    def test_givenRoom_whenNotify_thenNotifyCountIncremented(self):
        r = room_model.Room()

        self.mock_observer.notify(r)

        self.assertEqual(self.mock_observer.get_notify_count(), 1)


class TestTagPositionPublisher(unittest.TestCase):
    

    def setUp(self):
        self.position_publisher = room_model.TagPositionPublisher()


    def test_canBeNotify(self):
        r = room_model.Room()
        self.position_publisher.notify(r)

    
    def test_givenNewInstance_whenCheckSubclass_thenIsSubclassRoomObserver(self):
        self.assertTrue(issubclass(type(self.position_publisher), room_model.RoomObserver))


    def test_givenRoom_whenNotify_thenNotifyCountIncremented(self):
        r = room_model.Room()
        
        initial_notify_count = self.position_publisher.get_notify_count()
        self.position_publisher.notify(r)
        after_notify_count = self.position_publisher.get_notify_count()

        self.assertEqual(initial_notify_count + 1, after_notify_count)
        

    def test_givenRoom_whenNotify_thenPublishTagPoistion(self):
        r = room_model.Room()
        r.populate_with_default_anchor_and_tag()

        tag_pos_pub = room_model.TagPositionPublisher(room_model.MockMessageBroker())
        tag_pos_pub.notify(r)
        string_publisher = tag_pos_pub.get_message_broker()
        published_string = string_publisher.get_recent_publish()
        
        expected_string = f"{{\"source\": \"2000\", \"x\": 0.500, \"y\": 0.500, \"z\": 0.000}}"
        self.assertEqual(published_string, expected_string)


    def test_givenRoomWithTagWithNullPosition_whenNotify_thenPublishNothing(self):
        r = room_model.Room()

        tag_pos_pub = room_model.TagPositionPublisher(room_model.MockMessageBroker())
        tag_pos_pub.notify(r)
        string_publisher = tag_pos_pub.get_message_broker()
        published_string = string_publisher.get_recent_publish()

        self.assertEqual("", published_string)


class TestMessageBroker(unittest.TestCase):
    

    def test_givenMessageBroker_isAbstractClass(self):
        self.assertTrue(issubclass(room_model.MessageBroker, abc.ABC))


    def test_givenMessageBroker_canSetOnMessageCallback(self):
        set_on_message_callback_method = getattr(room_model.MessageBroker, 'set_on_message_callback')
        self.assertTrue(callable(set_on_message_callback_method))
        

    def test_givenMessageBroker_canPublish(self):
        publish_method = getattr(room_model.MessageBroker, 'publish')
        self.assertTrue(callable(publish_method))


class TestMockMessageBroker(unittest.TestCase):


    def setUp(self):
        self.mock_msg_broker = room_model.MockMessageBroker()
    
    
    def test_givenMockMessageBroker_whenCheckInstance_thenInstanceOfMessageBroker(self):
        self.assertTrue(isinstance(self.mock_msg_broker, room_model.MessageBroker))


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
  

class TestRedisMessageBroker(unittest.TestCase):
    
    
    def test_RedisMessageBroker_IsSubclassMessageBroker(self):
        self.assertTrue(issubclass(room_model.RedisMessageBroker, room_model.MessageBroker))


class TestRoomRangeUpdater(unittest.TestCase):


    def setUp(self):
        r = room_model.Room()
        r.populate_with_default_anchor_and_tag()
        mock_message_broker = room_model.MockMessageBroker()
        self.range_updater = room_model.RoomRangeUpdater(r, mock_message_broker)
    

    def test_givenRoom_whenInitialize_thenHasRoom(self):
        r = room_model.Room()
        mock_message_broker = room_model.MockMessageBroker()
        range_updater = room_model.RoomRangeUpdater(r, mock_message_broker)
        
        returned_room = range_updater.get_room()

        self.assertIs(r, returned_room)


    def test_givenMessageBroker_whenInitialize_thenOnMessageCallbackIsSet(self):
        r = room_model.Room()
        msg_broker = room_model.MockMessageBroker()

        range_updater = room_model.RoomRangeUpdater(r, msg_broker)

        returned_callback = msg_broker.get_on_message_callback()
        self.assertIsNotNone(returned_callback)


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
        mock_message_broker = room_model.MockMessageBroker()

        self.range_updater.set_message_broker(mock_message_broker)
        returned_message_broker = self.range_updater.get_message_broker()

        self.assertIs(mock_message_broker, returned_message_broker)


    def test_givenSetMessageBroker_whenMessageBrokerOnMessage_thenRoomIsUpdated(self):
        mock_message_broker = room_model.MockMessageBroker()
        r = self.range_updater.get_room()

        self.range_updater.set_message_broker(mock_message_broker)
        mock_message_broker.on_message("{\"source\": \"2000\", \"destination\": \"1000\", \"range\": \"420\"}")
        many_tag = r.get_many_tag()
        tag: room_model.Tag = many_tag[0]
        for dist_and_anchor in tag.get_many_dist_to_anchor_and_anchor():
            dist, anchor = dist_and_anchor
            if anchor.get_id() == "1000":
                break

        self.assertEqual(dist, 420)


class TestGoldenBehaviour(unittest.TestCase):
    

    def test_givenSensorData_thenReturnedPosition(self):
        r = room_model.Room()
        origin_id = "1000"
        origin_pos = trilaterate_algorithm.Position(0, 0, 0)
        far_x_id = "1001"
        far_x_pos = trilaterate_algorithm.Position(11, 0, 0)
        far_y_id = "1002"
        far_y_pos = trilaterate_algorithm.Position(0, 5, 0)
        tag_id = "2000"
        tag_name = "alice"
        tag_to_origin = math.sqrt(379)
        tag_to_far_x = math.sqrt(434)
        tag_to_far_y = math.sqrt(314)
        # Actual tag pos mathematically would be (3, 9, 17)
        # but for now algorithm is only for 2D space
        expected_tag_pos = trilaterate_algorithm.Position(3, 9, 0)

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

    