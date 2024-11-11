import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO
import time

# Set up GPIO pins for controlling pumps
inlet_pump_pin = 27  # Replace with the actual GPIO pin for the inlet pump
outlet_pump_pin = 22  # Replace with the actual GPIO pin for the outlet pump
ph_Up_pin = 23
ph_Down_pin = 24

GPIO.setmode(GPIO.BCM)
GPIO.setup(inlet_pump_pin, GPIO.OUT)
GPIO.setup(outlet_pump_pin, GPIO.OUT)
GPIO.setup(ph_Up_pin, GPIO.OUT)
GPIO.setup(ph_Down_pin, GPIO.OUT)

def on_message_hello(client, userdata, msg):
    print("Received hello message:", msg.payload.decode())

def on_message_water_level(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode()

    try:
        water_level = float(payload)
        print(f"Received water level: {water_level} lv")

        if water_level < 400:
            GPIO.output(outlet_pump_pin, GPIO.HIGH)
            GPIO.output(inlet_pump_pin, GPIO.LOW)
            print("Outlet Pump turned OFF")
            print("Inlet Pump turned ON")
        elif 400 <= water_level < 550:
            GPIO.output(outlet_pump_pin, GPIO.LOW)
            GPIO.output(inlet_pump_pin, GPIO.LOW)
            print("Outlet Pump turned ON")
            print("Inlet Pump turned ON")
        else:
            GPIO.output(outlet_pump_pin, GPIO.LOW)
            GPIO.output(inlet_pump_pin, GPIO.HIGH)
            print("Outlet Pump turned ON")
            print("Inlet Pump turned OFF")
        print('')
    except ValueError as e:
        print(f"Error parsing water level: {e}")

def on_message_Ph_level(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode()

    try:
        Ph_level = float(payload)
        print(f"Received Ph level: {Ph_level}")

        if Ph_level < 6.4:
            GPIO.output(ph_Up_pin, GPIO.LOW)
            GPIO.output(ph_Down_pin, GPIO.HIGH)
            print("Ph Up Pump turned ON")
            print("Ph Down Pump turned OFF")
        elif Ph_level > 8.1:
            GPIO.output(ph_Up_pin, GPIO.HIGH)
            GPIO.output(ph_Down_pin, GPIO.LOW)
            print("Ph Up Pump turned OFF")
            print("Ph Down Pump turned ON")
        else:
            GPIO.output(ph_Up_pin, GPIO.HIGH)
            GPIO.output(ph_Down_pin, GPIO.HIGH)
            print("Ph Up Pump turned OFF")
            print("Ph Down Pump turned OFF")
        print('')
    except ValueError as e:
        print(f"Error parsing Ph level: {e}")

# Set up client1 for WaterLevel and Ph topics
client1 = mqtt.Client()
client1.connect("broker.hivemq.com", 1883)

# Subscribe to the MQTT topics for client1
client1.subscribe("WaterLevel", qos=0)
client1.subscribe("Ph", qos=0)

# Assign callback functions for client1
client1.message_callback_add("WaterLevel", on_message_water_level)
client1.message_callback_add("Ph", on_message_Ph_level)

# Run client1 loop in the background
client1.loop_start()

# Set up client2 specifically for the HELLO topic
client2 = mqtt.Client()
client2.connect("broker.hivemq.com", 1883)

# Subscribe to the HELLO topic for client2
client2.subscribe("HELLO", qos=0)

# Assign callback function for client2
client2.message_callback_add("HELLO", on_message_hello)

# Run client2 loop
client2.loop_forever()