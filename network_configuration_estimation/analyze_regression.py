import pandas as pd
import statsmodels.api as sm
import numpy as np

def analyze_allreduce_performance(filename="all_data.csv"):
    """
    Performs multiple linear regression on the collected MPI performance data
    to estimate the fundamental cost parameters alpha (latency), beta (bandwidth), 
    and gamma (computation), keeping beta and gamma as separate variables (X2 and X3).
    
    WARNING: X2 and X3 are highly correlated, leading to potentially unstable 
    and inaccurate estimates for beta and gamma (high standard errors).
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
    
    # --- MODEL RE-SPECIFICATION: USE X1, X2, AND X3 SEPARATELY ---
    # The cost model is: T ≈ α*X1 + β*X2 + γ*X3
    X_train = df_train[['X1', 'X2', 'X3']].rename(columns={'X1': 'alpha_scaling_X1', 
                                                           'X2': 'beta_scaling_X2', 
                                                           'X3': 'gamma_scaling_X3'})
    Y_train = df_train['T']
    
    # --- 2. Perform Multiple Linear Regression on Training Data ---
    try:
        # The model is forced to fit T ≈ α*X1 + β*X2 + γ*X3
        model = sm.OLS(Y_train, X_train)
        results = model.fit()

        # --- 3. Extract Estimated Parameters ---
        # The coefficients directly correspond to alpha, beta, and gamma
        alpha_est = results.params['alpha_scaling_X1']
        beta_est = results.params['beta_scaling_X2']
        gamma_est = results.params['gamma_scaling_X3']

        # --- 4. Print Results ---
        print("\n" + "="*70)
        print("LINEAR ALLREDUCE COST MODEL REGRESSION RESULTS (Trained on 80% of data)")
        print("="*70)
        print("WARNING: X2 (2*m*(P-1)) and X3 (1*m*(P-1)) are highly multicollinear.")
        print("This regression attempts to fit alpha, beta, and gamma SEPARATELY.")
        print("The estimates for beta and gamma may be unstable/inaccurate.")
        print(f"Regression R-squared (Model Fit on Training Data): {results.rsquared:.4f}")
        print("\nEstimated Cost Parameters (Time per operation in seconds):")
        print("-" * 70)
        print(f"α (Latency per startup, from X1):     {alpha_est:.4e} seconds")
        print(f"β (Inverse Bandwidth, from X2):      {beta_est:.4e} seconds/word")
        print(f"γ (Computation Cost, from X3):       {gamma_est:.4e} seconds/word")
        print("-" * 70)

        # --- Calculate the combined factor K for comparison ---
        K_derived = 2.0 * beta_est + 1.0 * gamma_est
        print(f"CHECK: Combined K factor (2β + γ) is: {K_derived:.4e} seconds/word")
        
        # Print a short summary of the full regression output
        print("\nFull Regression Summary (Training Data) - Check Std. Errors on X2/X3:")
        print(results.summary().as_text())
        
        # --- 5. Test Prediction Accuracy on Hidden Test Data ---
        print("\n" + "="*70)
        print("MODEL PREDICTION VALIDATION (on 20% Hidden Data)")
        print("="*70)

        # Predict time (T) using the fitted model and the hidden test coefficients (X_test)
        X_test = df_test[['X1', 'X2', 'X3']].rename(columns={'X1': 'alpha_scaling_X1', 
                                                             'X2': 'beta_scaling_X2', 
                                                             'X3': 'gamma_scaling_X3'})
        Y_test_actual = df_test['T']
        
        model_preds = results.predict(X_test)
        
        # Calculate residuals (difference between predicted and actual)
        residuals = model_preds - Y_test_actual
        
        # Check if the residuals are small 
        max_abs_error = np.max(np.abs(residuals))
        mean_actual_time = np.mean(Y_test_actual)

        print(f"Mean Actual Test Time: {mean_actual_time:.4e} s")
        print(f"Max Absolute Prediction Error: {max_abs_error:.4e} s")

        if max_abs_error < mean_actual_time * 0.05: # Using a slightly larger threshold (5%) due to potential instability
            print(f"Validation Passed: Max absolute prediction error is acceptable relative to mean actual time.")
        else:
            print("Validation Warning: Max absolute prediction error is high. Check for outliers or non-linear effects.")


        # 6. Compare Predictions vs. Actual
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
