import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def plot_performance_data(filename="points_for_viz.csv"):
    """
    Reads performance data from a CSV file, focusing on 'm' (message size) 
    and 'T' (time), and generates a scatter plot.
    """
    try:
        # 1. Load the data
        df = pd.read_csv(filename)
        
    except FileNotFoundError:
        print(f"Error: The file '{filename}' was not found.")
        print("Please ensure 'points_for_viz.csv' is in the current directory.")
        return
    except Exception as e:
        print(f"An error occurred while reading the data: {e}")
        return

    # 2. Check if the required columns exist
    required_cols = ['m', 'T']
    if not all(col in df.columns for col in required_cols):
        print(f"Error: CSV must contain columns 'm' and 'T'. Available columns: {list(df.columns)}")
        return

    # 3. Create the plot
    plt.figure(figsize=(10, 6))
    
    # We plot T vs m. Using a scatter plot is usually best for performance points.
    # We also color points based on 'P' (process count), if it exists, for better analysis.
    if 'P' in df.columns:
        # Scatter plot colored by the process count 'P'. Marker size reduced to 15.
        scatter = plt.scatter(df['m'], df['T'], c=df['P'], cmap='viridis', s=15, alpha=0.7)
        plt.colorbar(scatter, label='Number of Processes (P)')
        plt.title('Allreduce Performance: Time vs. Message Size (Colored by P)')
    else:
        # Simple scatter plot if 'P' column is missing. Marker size reduced to 15.
        plt.scatter(df['m'], df['T'], s=15, color='royalblue', alpha=0.7)
        plt.title('Allreduce Performance: Time vs. Message Size')
    
    # Add axis labels and grid
    plt.xlabel('Message Size (m, in number of doubles)')
    plt.ylabel('Execution Time (T, in seconds)')
    plt.grid(True, linestyle='--', alpha=0.6)
    
    # Enhance readability for large 'm' values
    # if df['m'].max() > 10000:
    #     plt.ticklabel_format(axis='x', style='sci', scilimits=(0,0))

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_performance_data("points_for_viz.csv")