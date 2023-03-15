import os

def split_file(input_file):
    # Create a folder to store the output files
    os.makedirs("output", exist_ok=True)

    # Open the input file and read its contents
    with open(input_file, "r") as f:
        text = f.read()

    # Split the text into chunks of 20,000 words each
    words = text.split()
    chunks = [words[i:i+20000] for i in range(0, len(words), 20000)]

    # Write each chunk to a separate output file
    for i, chunk in enumerate(chunks):
        output_file = os.path.join("output", f"in{i+1}.txt")
        with open(output_file, "w") as f:
            f.write(" ".join(chunk))

    print(f"{len(chunks)} output files created in 'output' folder.")

# Example usage:
split_file("iliad.txt")
