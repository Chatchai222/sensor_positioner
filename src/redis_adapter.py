from redis import Redis
import time

class RedisPubSubAdapter:
    def __init__(self, hostname: str, port: int = 6379, auth: str = None):
        # Defining Class Variables
        self.PUBLISH_CHANNEL = "tag:motioncapture.coordinates"
        self.SUBSCRIBE_CHANNEL = "tag:motioncapture.uwb"
        
        # Initialize Variables
        self.hostname = hostname
        self.port = port
        self.auth = auth
        self.redis_callback = None

        self._redis = Redis(
            host = self.hostname,
            port = self.port,
            decode_responses = True
        )

        self.__redis_init()

    def __redis_init(self):
        self._redis.auth(password = self.auth)
        self._redis_pubsub = self._redis.pubsub()
        self._redis_pubsub.subscribe(**{self.SUBSCRIBE_CHANNEL: self.__redis_subscribe_callback}) 
        self._redis_pubsub_thread = self._redis_pubsub.run_in_thread()

        # Re-authentication is required because the first authentication
        # is already "consumed" by the PubSub Thread
        self._redis.auth(password = self.auth)

    def __redis_subscribe_callback(self, message):
        if self.callback:
            self.callback(message["data"])

    def set_callback(self, callback):
        self.callback = callback

    def publish(self, message: str):
        self._redis.publish(self.PUBLISH_CHANNEL, message)
        self._redis.set(self.PUBLISH_CHANNEL, message)

    def __del__(self):
        self._redis_pubsub_thread.stop()
