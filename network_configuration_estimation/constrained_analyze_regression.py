import pandas as pd
import statsmodels.api as sm
import numpy as np
from scipy.optimize import minimize

def analyze_allreduce_performance(filename="all_data.csv"):
    """
    Performs constrained multiple linear regression on the collected MPI performance data
    to estimate the fundamental cost parameters alpha (latency), beta (bandwidth), 
    and gamma (computation), with NON-NEGATIVITY CONSTRAINTS to ensure all parameters are positive.
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
    X_train = df_train[['X1', 'X2', 'X3']].values
    Y_train = df_train['T'].values
    
    # --- 2. Perform Constrained Least Squares Regression ---
    try:
        # Define the objective function (sum of squared residuals)
        def objective(params):
            alpha, beta, gamma = params
            predictions = alpha * X_train[:, 0] + beta * X_train[:, 1] + gamma * X_train[:, 2]
            residuals = Y_train - predictions
            return np.sum(residuals ** 2)
        
        # Define bounds for non-negativity constraints (all params >= 0)
        bounds = [(0, None), (0, None), (0, None)]  # (alpha >= 0, beta >= 0, gamma >= 0)
        
        # Initial guess (use unconstrained OLS as starting point)
        X_train_df = pd.DataFrame(X_train, columns=['X1', 'X2', 'X3'])
        model_initial = sm.OLS(Y_train, X_train_df).fit()
        initial_params = np.abs(model_initial.params.values)  # Use absolute values as initial guess
        
        # Perform constrained optimization
        result = minimize(objective, initial_params, method='L-BFGS-B', bounds=bounds)
        
        if not result.success:
            print(f"Warning: Optimization did not converge. Message: {result.message}")
        
        # Extract optimized parameters
        alpha_est, beta_est, gamma_est = result.x

        # Calculate R-squared manually for constrained model
        predictions_train = alpha_est * X_train[:, 0] + beta_est * X_train[:, 1] + gamma_est * X_train[:, 2]
        ss_res = np.sum((Y_train - predictions_train) ** 2)
        ss_tot = np.sum((Y_train - np.mean(Y_train)) ** 2)
        r_squared = 1 - (ss_res / ss_tot)

        # --- 4. Print Results ---
        print("\n" + "="*70)
        print("CONSTRAINED LINEAR ALLREDUCE COST MODEL REGRESSION RESULTS")
        print("="*70)
        print("Constraints Applied: alpha >= 0, beta >= 0, gamma >= 0")
        print(f"Regression R-squared (Model Fit on Training Data): {r_squared:.4f}")
        print("\nEstimated Cost Parameters (Time per operation in seconds):")
        print("-" * 70)
        print(f"α (Latency per startup, from X1):     {alpha_est:.4e} seconds")
        print(f"β (Inverse Bandwidth, from X2):      {beta_est:.4e} seconds/word")
        print(f"γ (Computation Cost, from X3):       {gamma_est:.4e} seconds/word")
        print("-" * 70)

        # --- Calculate the combined factor K for comparison ---
        K_derived = 2.0 * beta_est + 1.0 * gamma_est
        print(f"CHECK: Combined K factor (2β + γ) is: {K_derived:.4e} seconds/word")
        
        # --- 5. Test Prediction Accuracy on Hidden Test Data ---
        print("\n" + "="*70)
        print("MODEL PREDICTION VALIDATION (on 20% Hidden Data)")
        print("="*70)

        X_test = df_test[['X1', 'X2', 'X3']].values
        Y_test_actual = df_test['T'].values
        
        model_preds = alpha_est * X_test[:, 0] + beta_est * X_test[:, 1] + gamma_est * X_test[:, 2]
        
        # Calculate residuals (difference between predicted and actual)
        residuals = model_preds - Y_test_actual
        
        # Check if the residuals are small 
        max_abs_error = np.max(np.abs(residuals))
        mean_actual_time = np.mean(Y_test_actual)

        print(f"Mean Actual Test Time: {mean_actual_time:.4e} s")
        print(f"Max Absolute Prediction Error: {max_abs_error:.4e} s")

        if max_abs_error < mean_actual_time * 0.05:
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
    analyze_allreduce_performance("points_for_viz.csv")