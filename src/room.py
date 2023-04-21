import trilaterate
import math
import json

class Anchor:
    

    def __init__(self, id: str, pos: trilaterate.Position):
        self._id = id
        self._name = "AnchorSensor_stud_name"
        self._pos = pos


    def __eq__(self, other_pos):
        return self._id == other_pos.get_id()
    

    def __repr__(self):
        return f"id: {self._id}, name: {self._name}, pos: {self._pos}"
        

    def is_equal(self, other_anchor):
        return self._id == other_anchor.get_id()


    def get_id(self):
        return self._id


    def get_name(self):
        return self._name
    

    def get_position(self):
        return self._pos
    

    def set_name(self, in_name: str):
        self._name = in_name


    def set_position(self, in_pos: trilaterate.Position):
        self._pos = in_pos


class Tag:
    

    _pos_calculator = trilaterate.FarAxisOrigin2DPositionCalculator()


    # classmethod can access class attribute but not instance
    @classmethod
    def get_position_calculator(self):
        return self._pos_calculator
    

    def __init__(self, id: str):
        self._id = id
        self._name = "TagSensor_stud_name"
        self._many_dist_to_anchor_and_anchor = []


    def get_id(self):
        return self._id


    def get_name(self) -> str:
        return self._name
    

    def set_name(self, in_name: str):
        self._name = in_name


    def get_many_dist_to_anchor_and_anchor(self):
        return self._many_dist_to_anchor_and_anchor
    

    def upsert_dist_to_anchor_and_anchor(self, in_dist_to_anchor, in_anchor):
        is_dist_updated = False
        for i in range(len(self._many_dist_to_anchor_and_anchor)):
            _, anchor = self._many_dist_to_anchor_and_anchor[i]
            if anchor.is_equal(in_anchor):
                self._many_dist_to_anchor_and_anchor[i][0] = in_dist_to_anchor
                is_dist_updated = True
                break

        if not is_dist_updated:
            self._many_dist_to_anchor_and_anchor.append([in_dist_to_anchor, in_anchor])  
            

    def get_position(self):
        many_dist_to_anchor_and_anchor_pos = []
        for dist, anchor in self._many_dist_to_anchor_and_anchor:
            many_dist_to_anchor_and_anchor_pos.append( [dist, anchor.get_position()])
        
        return self.__class__._pos_calculator.get_position_or_null_position(many_dist_to_anchor_and_anchor_pos)
    

    def __eq__(self, other):
        return self._id == other.get_id()
    

    def __repr__(self):
        return f"id: {self._id}, name: {self._name}, many_dist_to_anchor_and_anchor: {self._many_dist_to_anchor_and_anchor}"


class AnchorCollection:


    def __init__(self):
        self._many_anchor = []

    
    def get_many_anchor(self):
        return self._many_anchor


    def insert_anchor_id_and_pos(self, in_anchor_id, in_anchor_pos):
        anchor = Anchor(in_anchor_id, in_anchor_pos)
        if anchor in self._many_anchor:
            raise self.__class__.InsertSameAnchorIdException("Insert anchor with same id")
        else:
            self._many_anchor.append(anchor)
            

    def update_anchor_position(self, in_anchor_id, in_anchor_pos):
        in_anchor = Anchor(in_anchor_id, in_anchor_pos)
        for i, anchor in enumerate(self._many_anchor):
            if anchor.get_id() == in_anchor_id:
                self._many_anchor[i] = in_anchor
                return

        raise self.__class__.UpdateNonexistentAnchorIdException("Update anchor with nonexistent id")


    def get_anchor(self, in_anchor_id):
        for anchor in self._many_anchor:
            if anchor.get_id() == in_anchor_id:
                return anchor
            
        raise self.__class__.GetNonexistentAnchorIdException


    class InsertSameAnchorIdException(Exception):
        pass

    
    class UpdateNonexistentAnchorIdException(Exception):
        pass


    class GetNonexistentAnchorIdException(Exception):
        pass


class TagCollection:
    

    def __init__(self):
        self._many_tag = []
        
    
    def get_many_tag(self):
        return self._many_tag
    

    def insert_tag_by_id(self, in_tag_id):
        in_tag = Tag(in_tag_id)
        if in_tag in self._many_tag:
            raise self.__class__.InsertSameTagIdException
        else:
            self._many_tag.append(in_tag)

    
    def get_tag(self, in_tag_id):
        for tag in self._many_tag:
            if tag.get_id() == in_tag_id:
                return tag
            
        raise self.__class__.GetNonexistentTagIdException
    
    
    def __repr__(self):
        return f"many_tag: {self._many_tag}"
    

    class InsertSameTagIdException(Exception):
        pass

    
    class GetNonexistentTagIdException(Exception):
        pass
    

class Room:

    
    def __init__(self):
        self._anchor_collection = AnchorCollection()
        self._tag_collection = TagCollection()
        self._many_observer = []


    def populate_with_default_anchor_and_tag(self):
        self.upsert_anchor_position("1000", trilaterate.Position(0, 0, 0))
        self.upsert_anchor_position("1001", trilaterate.Position(0.5, 0, 0))
        self.upsert_anchor_position("1002", trilaterate.Position(0, 0.5, 0))

        self.upsert_tag_to_anchor_dist("2000", "1000", math.sqrt(0.5))
        self.upsert_tag_to_anchor_dist("2000", "1001", 0.5)
        self.upsert_tag_to_anchor_dist("2000", "1002", 0.5)


    def get_many_anchor(self):
        return self._anchor_collection.get_many_anchor()
    
    
    def upsert_anchor_position(self, in_anchor_id: str, in_anchor_pos: trilaterate.Position):
        try:
            self._anchor_collection.update_anchor_position(in_anchor_id, in_anchor_pos)
        except AnchorCollection.UpdateNonexistentAnchorIdException:
            self._anchor_collection.insert_anchor_id_and_pos(in_anchor_id, in_anchor_pos)

        self.notify_observer()
    

    def update_anchor_name(self, in_anchor_id: str, in_name: str):
        try:
            anchor = self._anchor_collection.get_anchor(in_anchor_id)
            anchor.set_name(in_name)
            self.notify_observer()
        except AnchorCollection.GetNonexistentAnchorIdException:
            pass
    

    def get_many_tag(self):
        return self._tag_collection.get_many_tag()
    

    def upsert_tag_to_anchor_dist(self, in_tag_id: str, in_anchor_id: str, in_dist: float):
        try:
            anchor = self._anchor_collection.get_anchor(in_anchor_id)
        except AnchorCollection.GetNonexistentAnchorIdException:
            return
        
        try:
            tag = self._tag_collection.get_tag(in_tag_id)
        except TagCollection.GetNonexistentTagIdException:
            self._tag_collection.insert_tag_by_id(in_tag_id)
            tag = self._tag_collection.get_tag(in_tag_id)

        tag.upsert_dist_to_anchor_and_anchor(in_dist, anchor)
        self.notify_observer()


    def update_tag_name(self, in_tag_id, in_tag_name):
        try:
            tag = self._tag_collection.get_tag(in_tag_id)
            tag.set_name(in_tag_name)
            self.notify_observer()
        except TagCollection.GetNonexistentTagIdException:
            pass


    def get_many_observer(self) -> list:
        return self._many_observer


    def add_observer(self, in_observer):
        self._many_observer.append(in_observer)


    def notify_observer(self):
        for each_observer in self._many_observer:
            each_observer.notify(self)
        

class TagToJSONStringConverter:
    
    def __init__(self):
        pass


    def get_JSON(self, tag: Tag) -> str:
        tag_id = tag.get_id()
        tag_pos = tag.get_position()

        if type(tag_pos) == trilaterate.NullPosition:
            raise self.__class__.TagHasNullPositionException
        
        x_pos = tag_pos.get_x()
        y_pos = tag_pos.get_y()
        z_pos = tag_pos.get_z()

        return f"\"source\": \"{tag_id}\", \"x\": {x_pos}, \"y\": {y_pos}, \"z\": {z_pos}"


    class TagHasNullPositionException(Exception):
        pass
    

class MockMessageBroker:
    

    def __init__(self):
        self._recent_publish = ""
        self._on_message_callback = None


    def publish(self, in_string: str):
        self._recent_publish = in_string


    def get_recent_publish(self) -> str:
        return self._recent_publish


    def set_on_message_callback(self, in_callback):
        self._on_message_callback = in_callback
        

    def get_on_message_callback(self):
        return self._on_message_callback
    

    def on_message(self, in_message):
        if self._on_message_callback is not None:
            self._on_message_callback(in_message)
        


class TagPositionPublisher:

    
    def __init__(self, in_message_broker=MockMessageBroker()):
        self._message_broker = in_message_broker
        self._tag_to_string_converter = TagToJSONStringConverter()
        self._notify_count = 0


    def notify(self, in_room: Room):
        self._notify_count += 1

        many_tag = in_room.get_many_tag()
        for each_tag in many_tag:
            try:
                json_string = self._tag_to_string_converter.get_JSON(each_tag)
                self._message_broker.publish(json_string)
            except self._tag_to_string_converter.__class__.TagHasNullPositionException:
                pass


    def get_notify_count(self):
        return self._notify_count
    

    def get_message_broker(self):
        return self._message_broker

    
class RoomRangeUpdater:


    def __init__(self, in_room: Room, in_message_broker=MockMessageBroker()):
        self._room = in_room
        self._message_broker = in_message_broker


    def get_room(self) -> Room:
        return self._room
    

    def update_room(self, in_str):
        msg_dict = json.loads(in_str)

        tag_id = msg_dict["source"]
        anchor_id = msg_dict["destination"]
        dist = float(msg_dict["range"])

        self._room.upsert_tag_to_anchor_dist(tag_id, anchor_id, dist)


    def set_message_broker(self, in_message_broker):
        self._message_broker = in_message_broker
        on_msg_callback = lambda in_msg: self.update_room(in_msg)
        self._message_broker.set_on_message_callback(on_msg_callback)


    def get_message_broker(self):
        return self._message_broker
    