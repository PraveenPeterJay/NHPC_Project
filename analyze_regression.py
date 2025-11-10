import pandas as pd
import statsmodels.api as sm
import numpy as np
import test1

# --- NOTE: You must first run the C code multiple times and combine the outputs: ---
# smpirun -np 4 ./a.out > data_P4.csv
# smpirun -np 8 ./a.out > data_P8.csv
# smpirun -np 16 ./a.out > data_P16.csv
# cat data_P4.csv data_P8.csv data_P16.csv | grep -v 'P,m' > all_data.csv

def analyze_allreduce_performance(filename="all_data.csv"):
    """
    Performs multiple linear regression on the collected MPI performance data
    to estimate the fundamental cost parameters alpha (latency) and the composite 
    bandwidth/computation factor (K_BW_Comp).
    
    NOTE: X2 and X3 are too correlated (multicollinear) to separate beta and gamma. 
    We use X1 for alpha and X3 as a combined scaling factor for the m*(P-1) term.
    """
    try:
        # Load the combined data
        raw_df = pd.read_csv(filename)
        
    except FileNotFoundError:
        print(f"Error: Data file '{filename}' not found.")
        print("Please ensure you have run the C code with multiple process counts (P) ")
        print("and combined the output into a single file named 'all_data.csv'.")
        return
    except Exception as e:
        print(f"An error occurred while reading the data: {e}")
        return

    # --- DATA SPLITTING: Hide 20% of points for testing (randomly selected) ---
    TEST_FRACTION = 0.20
    
    # Generate random indices for reproducibility
    np.random.seed(42)
    shuffled_indices = np.random.permutation(len(raw_df))
    
    n_test = int(TEST_FRACTION * len(raw_df))
    
    # Split indices into test and train sets
    test_indices = shuffled_indices[:n_test]
    train_indices = shuffled_indices[n_test:]
    
    # Split the DataFrame into training and testing sets
    df_train = raw_df.iloc[train_indices].copy()
    df_test = raw_df.iloc[test_indices].copy()
    
    # --- MODEL RE-SPECIFICATION: Use X1 (alpha) and X3 (combined m scaling) ---
    # Due to high multicollinearity between X2 (2*(P-1)m) and X3 (1*(P-1)m), 
    # we must drop X2 and use X3 as the single 'm-scaling' variable (X_scaling).
    X_train = df_train[['X1', 'X3']].rename(columns={'X3': 'X_scaling'})
    Y_train = df_train['T']
    
    # --- 2. Perform Multiple Linear Regression on Training Data ---
    try:
        model = sm.OLS(Y_train, X_train)
        results = model.fit()

        # --- 3. Extract Estimated Parameters ---
        alpha_est = results.params['X1']
        K_bw_comp_est = results.params['X_scaling']

        # --- 4. Print Results ---
        print("\n" + "="*50)
        print("LINEAR ALLREDUCE COST MODEL REGRESSION RESULTS (Trained on 80% of data)")
        print("="*50)
        print("NOTE: Multicollinearity requires combining beta and gamma into one term K.")
        print("K represents the combined cost: K = 2*β + 1*γ.")
        print(f"Regression R-squared (Model Fit on Training Data): {results.rsquared:.4f}")
        print("\nEstimated Cost Parameters (Time per operation in seconds):")
        print("-" * 50)
        print(f"α (Latency per startup):        {alpha_est:.4e} seconds")
        print(f"K (Composite m*(P-1) factor): {K_bw_comp_est:.4e} seconds/word")
        print("-" * 50)

        # --- Derive Beta and Gamma using the assumption ---
        # Assumption: For communication-bound collectives, computation (gamma) is often negligible 
        # compared to communication (beta). We assume gamma is zero to derive beta.
        # If gamma = 0, then K = 2*beta, so beta = K / 2.
        beta_derived = K_bw_comp_est / 2.0
        gamma_derived = 0.0 

        print("\nDerived and Separated Parameters (Assuming γ ≈ 0):")
        print("This is the required separation, based on K = 2*β + γ where γ is negligible.")
        print("-" * 50)
        print(f"Derived β (Inv. Bandwidth):    {beta_derived:.4e} seconds/word")
        print(f"Derived γ (Computation):       {gamma_derived:.4e} seconds/word (Assumed)")
        print("-" * 50)
        
        # Print a short summary of the full regression output
        print("\nFull Regression Summary (Training Data):")
        print(results.summary().as_text())
        
        # --- 5. Test Prediction Accuracy on Hidden Test Data ---
        print("\n" + "="*50)
        print("MODEL PREDICTION VALIDATION (on 20% Hidden Data)")
        print("="*50)

        # Predict time (T) using the fitted model and the hidden test coefficients (X_test)
        # Use X1 and X3 (renamed) for prediction
        X_test = df_test[['X1', 'X3']].rename(columns={'X3': 'X_scaling'})
        Y_test_actual = df_test['T']
        
        model_preds = results.predict(X_test)
        
        # Calculate residuals (difference between predicted and actual)
        residuals = model_preds - Y_test_actual
        
        # Check if the residuals are small 
        max_abs_error = np.max(np.abs(residuals))
        mean_actual_time = np.mean(Y_test_actual)

        print(f"Mean Actual Test Time: {mean_actual_time:.4e} s")
        print(f"Max Absolute Prediction Error: {max_abs_error:.4e} s")

        if max_abs_error < mean_actual_time * 0.01:
            print(f"Validation Passed: Max absolute prediction error is less than 1% of mean actual time.")
        else:
            print("Validation Warning: Max absolute prediction error is high. Check for outliers or non-linear effects.")


        # 6. Compare with Analytical Model (using external 'test1' module)
        procs = df_test['P']
        messages = df_test['m']
        
        # Use the derived beta and gamma for the analytical check
        beta_est_approx = beta_derived
        gamma_est_approx = gamma_derived

        print("\n--- Hidden Test Point Predictions vs. Actual Time ---")
        print("Model Pred | Actual Time | Model Residual")
        print("-----------------------------------------------------")
        
        for (model, actual) in zip(model_preds, Y_test_actual):
            residual = model - actual
            print(f"{model:.4e} | {actual:.4e} | {residual:.4e}")

    except Exception as e:
        print(f"\nAn error occurred during regression analysis: {e}")
        print("Check your data for non-numeric values or constant columns.")
        
if __name__ == "__main__":
    
    analyze_allreduce_performance("linear_allreduce_data_points.csv")
