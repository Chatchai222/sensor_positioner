import room_model
import trilaterate_algorithm

def main():
    print("Setting up the trilateration program")
    redis_msg_broker = room_model.RedisMessageBroker("192.168.4.33", auth="ictadmin")
    r = room_model.Room()
    tag_pos_pub = room_model.TagPositionPublisher(redis_msg_broker)
    room_range_updater = room_model.RoomRangeUpdater(r, redis_msg_broker)
    
    r.upsert_anchor_position("1000", trilaterate_algorithm.Position(0.00, 0.00, 0.00))
    r.upsert_anchor_position("1001", trilaterate_algorithm.Position(5.40, 0.00, 0.00))
    r.upsert_anchor_position("1002", trilaterate_algorithm.Position(0.00, 9.30, 0.00))
    r.add_observer(tag_pos_pub)
    
    print("Ending of setup the trilateration program")
    print("Entering infinite loop to keep trilateration program running")
    while(True):
        pass

   

if __name__ == "__main__":
    main()