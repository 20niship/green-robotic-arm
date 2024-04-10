import os 
import glob

def remove_trailing_comma(input_file, output_file):
    with open(input_file, 'r') as infile:
        lines = infile.readlines()

    print(lines[0][-1], lines[0][-2])
    # 行末のカンマを除去
    updated_lines = [line[:-2] + "\n" if line.endswith(',\n') else line for line in lines]

    with open(output_file, 'w') as outfile:
        outfile.writelines(updated_lines)


files = glob.glob('dataset/exp2/*.csv')

for file in files:
    print(file)
    outfile = file.replace('exp2', 'exp2_csv')
    remove_trailing_comma(file, outfile)

