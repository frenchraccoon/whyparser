#!/usr/bin/python

import sys
import heapq

def main():
  # parser.py distinct [--from TIMESTAMP] [--to TIMESTAMP] input_file
  # parser.py top nb_top_queries [--from TIMESTAMP] [--to TIMESTAMP] input_file

  rfrom = 0
  rto = sys.maxsize
  dest = None

  # get opptional from/to
  for arg in sys.argv:
    if arg == '--from' or arg == '--to':
      dest = 'r' + arg[2:]
    elif dest != None:
      value = int(arg)
      exec(dest + '=' + str(value))
      dest = None

  # read file
  hash = {}
  fname = sys.argv[len(sys.argv) - 1]
  with open(fname) as f:
    for line in f:
      line = line.strip()
      arr = line.split('\t')
      if len(arr) == 2:
        try:
          ts = int(arr[0])
          if ts >= rfrom and ts <= rto:
            if arr[1] in hash:
              hash[arr[1]] += 1
            else:
              hash[arr[1]] = 1
        except ValueError:
          pass

  # output result
  if sys.argv[1] == 'distinct':
    print("{num}".format(num=len(hash)))
  elif sys.argv[1] == 'top':
    top = int(sys.argv[2])
    heap = []

    # using regular order ; number of digits is supposed fixed for the tests
    for k, v in hash.iteritems():
      s = str(v).zfill(10) + '\t' + k
      if len(heap) < top:
        heapq.heappush(heap, s)
      elif heap[0] < s:
        heapq.heappop(heap)
        heapq.heappush(heap, s)

    # extract
    list = []
    while len(heap) != 0:
      list.append(heapq.heappop(heap))

    # print reversed list
    for s in list[::-1]:
      arr = s.split('\t')
      print(arr[1] + ' ' + str(int(arr[0])))
  else:
    raise Exception("unknown mode: " + sys.argv[1])

if __name__== "__main__":
  main()
