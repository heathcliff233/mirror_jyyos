#!/usr/bin/env python3

from itertools import combinations_with_replacement

V = {'x': 0}
P = [ [f't{t} = x', f'x = t{t} + 1'] * m for t, m in enumerate([4, 4]) ]

print('Model checking:')
for t, ops in enumerate(P):
  print(f'T{t+1}:', '; '.join(ops))

I = [ [] ]
for ops in P:
  I_new, l = [], len(I[0]) + 1
  for ins_pos in combinations_with_replacement(range(l), len(ops)):
    for sched in I:
      new_sched = []
      for j in range(l):
        new_sched += [op for k, op in enumerate(ops) if ins_pos[k] == j]
        if j < l - 1: new_sched.append(sched[j])
      I_new.append(new_sched)
  I = I_new

print(f'Found {len(I)} schedules')
for sched in I:
  p = '; '.join([f'{var} = {val}' for var, val in V.items()] + sched)
  exec(p)
  print(', '.join([f'{var} = {globals()[var]}' for var in V]), '|', p)
