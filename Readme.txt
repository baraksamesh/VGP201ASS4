We had a few problems difficult to approach to:

1) Face Direction of movement - For Bazier food, We tried to make the cube face the direction of movement.
We found something that is called "LookAt Matrix" which is used for transforming cameras
to look at a certain point from a certain posision. After creating the matrix by guidence
of an article found on the web (https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function)
we needed a way of applying the transformation given the Movable.h interface was needed.
It tok a lot of failed attempts, but the way that worked for us was figuring the euler angles needed to immitate the
otation of the LookAt matrix by 3 different rotations (https://www.geometrictools.com/Documentation/EulerAngles.pdf),
one for each axis, and then using MyRotate with those angles.
Now that we can get an object to point at a direction given a Vector3f, each frame we find the direction of movement 
of the food and apply LookAt function on it so it faces the direction of movement.

2) Squeezing cubes - Implementing bouncy food (kinematic movement affected by gravity) was the low hanging fruit,
but it did not yet felt right. The required behaviour we wanted was giving the bouncy cubes a "jelly-like" behaviour,
hiding it's boring rigid nature. We wound up applying direct manipulations on the V matrix of the bouncy cube- by a
given factor>1, it streches on the Y axis and shrinks on X and Z axes, and vise-versa given a factor<1.
Then, each frame we calculate a factor as a function of the current height of the cube, the current vertical speed and a lot 
of magic numbers and applying a manipulation on V of the cube, making it strech vertically on high vertical speed and
squash when close to the ground.

3)