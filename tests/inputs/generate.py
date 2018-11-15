#!/usr/bin/python3

"""
This ugly piece of code generates random input for the simplifier.
"""

import random
import sys

def random_id(length):
    alpha = 'abcdefghijklmnopqrstuvwxyz'
    id = ''
    for i in range(0,length):
        id += random.choice(alpha)
    return id

if len(sys.argv) == 1:
    print("Missing arguments:")
    print("First argument - number of people")
    print("Second argument - number of transactions")
    exit(1)

people = int(sys.argv[1])
transactions = int(sys.argv[2])

print("def currency czk")
people = [random_id(12) for _ in range(people)]
for p in people:
    print(f"def person {p}")

def rp():
    return random.choice(people)

def rv():
    return random.random()*1000

for i in range(transactions):
    print(f"{rp()} paid {rv()}czk for {rp()}")


