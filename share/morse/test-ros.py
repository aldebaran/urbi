from morse.builder.morsebuilder import *

# Append ATRV robot to the scene
atrv = Robot('atrv')

# Append an actuator
motion = Actuator('v_omega')
motion.translate(z=0.3)
atrv.append(motion)

# Append an odometry sensor
odometry = Sensor('odometry')
odometry.translate(x=-0.1, z=0.83)
atrv.append(odometry)

# Append a proximity sensor
proximity = Sensor('proximity')
proximity.translate(x=-0.2, z=0.83)
atrv.append(proximity)

# Append a Pose sensor (GPS + Gyroscope)
pose = Sensor('pose')
pose.translate(x=0.2,z=0.83)
atrv.append(pose)

# Append a sick laser
sick = Sensor('sick')
sick.translate(x=0.18,z=0.94)
atrv.append(sick)
sick.properties(resolution = 1)
sick.properties(laser_range = 5.0)

# Append a camera
cam = Sensor('video_camera')
cam.translate(x=0.3,z=1.1)
atrv.append(cam)
cam.properties(cam_width = 128, cam_height = 128)

# This has a weird behavior with the Robot.
# Append a accelerometer.
#accelerometer = Sensor('accelerometer')
#accelerometer.translate(x=0.1,z=0.83)
#atrv.append(accelerometer)

# Append a gps.
gps = Sensor('gps')
gps.translate(x=0.1,z=0.83)
atrv.append(gps)

# Configure the middlewares
gps.configure_mw('ros')
#accelerometer.configure_mw('ros')
motion.configure_mw('ros')
odometry.configure_mw('ros')
proximity.configure_mw('ros')
pose.configure_mw('ros')
sick.configure_mw('ros')
cam.configure_mw('ros')


# Configure the middlewares
motion.configure_mw('socket')
pose.configure_mw('socket')

env = Environment('laas/grande_salle')
env.aim_camera([1.0470, 0, 0.7854])
