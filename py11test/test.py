import kf_cpp
import json

# simple functions --int
print('add(8,9)=', kf_cpp.add(8, 9))
print('add_arg(8,9)=', kf_cpp.add_arg(j=8, i=9))
# print('add_default(8,7)=', kf_cpp.add_default())

print('sub(8,9)=', kf_cpp.sub(8, 9))

# simple functions --bool
print('judge(8,9)=', kf_cpp.judge(8, 9))
print('judge(8,0)=', kf_cpp.judge(8, 0))

# simple attributes
print('kf_cpp.age=',kf_cpp.age)
print('kf_cpp.gender=',kf_cpp.gender)
print('kf_cpp.name=',kf_cpp.name)

# class
Sam = kf_cpp.Hello()
print('Sam.say(): ')
Sam.say('my name is Sam')

# struct Pet
pet = kf_cpp.Pet("cat")
print('pet.getName(): ',pet.getName())

#
print('kf_cpp.sumsum(3)=',kf_cpp.sumsum(3))
print('kf_cpp.addpoint=',kf_cpp.addpoint())


# val3dity
path = "/home/ziyanwu/data/cityjson/cube.json"
with open(path, 'r') as f:
    cityjson = json.load(f)
print('type(cityjson): ', type(cityjson))

print(kf_cpp.vc(cityjson,0.001,0.01,20.0,-1.0))
