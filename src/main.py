import room
import redis_message_broker

def main():
    redis_msg_broker = redis_message_broker.RedisMessageBroker("localhost")
    r = room.Room()
    tag_pos_pub = room.TagPositionPublisher(redis_msg_broker)
    room_range_updater = room.RoomRangeUpdater(r)

    r.add_observer(tag_pos_pub)
    room_range_updater.set_message_broker(redis_msg_broker)

    while(True):
        pass

    


   

if __name__ == "__main__":
    main()