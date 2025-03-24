import sys
import random

def generate_matrix(n, filename):
    with open(filename, 'w') as f:
        for _ in range(n):
            row = [str(random.randint(0, 9)) for _ in range(n)]
            f.write(" ".join(row) + "\n")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python generate_matrix.py <n> <output_file>")
        sys.exit(1)

    n = int(sys.argv[1])
    output_file = sys.argv[2]
    generate_matrix(n, output_file)