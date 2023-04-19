import unittest
import math

import trilaterate
import room


class TestAnchorSensor(unittest.TestCase):
    
    
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


class TestTagSensor(unittest.TestCase):
    

    def setUp(self):
        self.tag = room.Tag("2000")


    def test_canInitializeWithId(self):
        tag_sensor = room.Tag("2000")
    

    def test_newTag_hasStudName(self):
        self.assertEqual(self.tag.get_name(), "TagSensor_stud_name")
    

    def test_canSetAndGetName(self):
        self.tag.set_name("mytag1")
        
        self.assertEqual(self.tag.get_name(), "mytag1")


    def test_classCanGetPositionCalculator(self):
        pos_calculator = room.Tag.get_position_calculator()
        pos_calculator_type = type(pos_calculator)
        
        self.assertTrue(issubclass(pos_calculator_type, trilaterate.PositionCalculator))

    
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
        
        
    def test_givenExistedAnchor_whenUpdateAnchorName_thenAnchorNameUpdated(self):
        self.room.upsert_anchor_position("10", trilaterate.Position(10, 20, 30))

        self.room.update_anchor_name("10", "myanchorname")
        anchor = self.room.get_many_anchor()[0]

        self.assertEqual(anchor.get_name(), "myanchorname")


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


    def test_givenUpdateTagName_whenNoTagExist_thenDoNothing(self):
        tag_id = "2000"
        tag_name = "alice"
        
        self.room.update_tag_name(tag_id, tag_name)
        many_tag = self.room.get_many_tag()
        
        self.assertEqual(many_tag, [])


    def test_givenUpdateTagName_whenTagIdMatch_thenUpdateTagName(self):
        tag_id = "2000"
        tag_name = "alice"
        tag_to_anchor_dist = "8.32"
        anchor_id = "1000"
        anchor_pos = trilaterate.Position(10, 20, 30)

        self.room.upsert_anchor_position(anchor_id, anchor_pos)
        self.room.upsert_tag_to_anchor_dist(tag_id, anchor_id, tag_to_anchor_dist)
        self.room.update_tag_name(tag_id, tag_name)
        many_tag = self.room.get_many_tag()
        tag = many_tag[0]

        self.assertEqual(tag.get_name(), tag_name)


    def test_givenNewRoom_whenGetManyObserver_thenManyObserverEmpty(self):
        many_observer = self.room.get_many_observer()

        self.assertEqual([], many_observer)
                 
        
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
        
        self.assertRaises(room.TagToJSONStringConverter.TagHasNullPositionException, self.converter.get_JSON, tag)
        
        
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
        

class TagPositionPublisher(unittest.TestCase):

    
    pass


        



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

    