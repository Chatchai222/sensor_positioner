import room
import redis_message_broker

def main():
    print("Setting up the trilateration program")
    redis_msg_broker = redis_message_broker.RedisMessageBroker("192.168.4.33", auth="ictadmin")
    r = room.Room()
    tag_pos_pub = room.TagPositionPublisher(redis_msg_broker)
    room_range_updater = room.RoomRangeUpdater(r, redis_msg_broker)
    
    r.populate_with_default_anchor_and_tag()
    r.add_observer(tag_pos_pub)
    
    print("Ending of setup the trilateration program")
    print("Entering infinite loop to keep trilateration program running")
    while(True):
        pass

   

if __name__ == "__main__":
    main()