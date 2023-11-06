import sys
import argparse
import pandas as pd

def parse_arg(args):
  parser = argparse.ArgumentParser(prog='paris calib reader', description='Read Paris Calibration')
  parser.add_argument('file', type=str, help='name of the file to read')
  # parser.add_argument('-n', type=str, help='name of the file to read')
  return parser.parse_args(args)

def main():
  args = sys.argv[1:]
  pargs = parse_arg(args)

  print("reading : "+pargs.file)

  df = pd.read_csv(pargs.file,sep=';')
  print(df['511'][0])
  
  data = [pd.DataFrame]
  calibPoints = [int]
  
  i = -1
  for a in df.columns:
    i+=1
    try:
      int(a)
      # calibPoints[i] = int(a)
      # print(a)
    except:
      print("Column "+str(a)+" is not a column holding data")
      continue
    data.insert(i, df[a])
      
  print(data)
  
if __name__ == "__main__":
  main()