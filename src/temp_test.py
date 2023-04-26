import redis_message_broker

def on_message_callback(in_message):
    print("Message got from redis is: ")
    print(in_message)

redis_msg_broker = redis_message_broker.RedisMessageBroker("192.168.4.33", auth="ictadmin")
redis_msg_broker.set_on_message_callback(on_message_callback)

while True:
    pass

