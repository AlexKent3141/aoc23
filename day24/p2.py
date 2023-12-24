import sympy as sym

# Throw loc
x0 = sym.Symbol('x0')
y0 = sym.Symbol('y0')
z0 = sym.Symbol('z0')

# Throw vel
i0 = sym.Symbol('i0')
j0 = sym.Symbol('j0')
k0 = sym.Symbol('k0')

# Time stamps for intersections.
t1 = sym.Symbol('t1')
t2 = sym.Symbol('t2')
t3 = sym.Symbol('t3')

# Constants from our input data.
input1 = "200027938836082 135313515251542 37945458137479 133 259 506".split()
x1 = int(input1[0])
y1 = int(input1[1])
z1 = int(input1[2])
i1 = int(input1[3])
j1 = int(input1[4])
k1 = int(input1[5])

input2 = "285259862606823 407476720802151 448972585175416 12 -120 -241".split()
x2 = int(input2[0])
y2 = int(input2[1])
z2 = int(input2[2])
i2 = int(input2[3])
j2 = int(input2[4])
k2 = int(input2[5])

input3 = "329601664688534 370686722303193 178908568819244 -133 -222 168".split()
x3 = int(input3[0])
y3 = int(input3[1])
z3 = int(input3[2])
i3 = int(input3[3])
j3 = int(input3[4])
k3 = int(input3[5])

sol = sym.solve((
  x0 + i0 * t1 - x1 - i1 * t1,
  x0 + i0 * t2 - x2 - i2 * t2,
  x0 + i0 * t3 - x3 - i3 * t3,
  y0 + j0 * t1 - y1 - j1 * t1,
  y0 + j0 * t2 - y2 - j2 * t2,
  y0 + j0 * t3 - y3 - j3 * t3,
  z0 + k0 * t1 - z1 - k1 * t1,
  z0 + k0 * t2 - z2 - k2 * t2,
  z0 + k0 * t3 - z3 - k3 * t3),
  (x0, y0, z0, i0, j0, k0, t1, t2, t3))

print(sol)

print(sol[0][0] + sol[0][1] + sol[0][2])
