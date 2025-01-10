import numpy as np
import matplotlib.pyplot as plt
import math
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d import Axes3D
import torch
import torch.nn as nn
import torch.optim as optim
import os

########################################
# 1. Global Constants and Hyperparams
########################################

GRAVITY = 9.82          # m/s^2
TIME_STEP = 0.01        # seconds
TOLERANCE = 0.1        # Interception tolerance (meters)
TARGET_RADIUS = 4     # Radius for random target generation

# Rocket parameters
WEIGHT = 0.75           # kg
THRUST = 6.0            # N
DRAG_COEFF = 0.8        # Dimensionless drag coefficient

# RL Training parameters
NUM_EPISODES = 500      # Adjust as needed
LEARNING_RATE = 1e-3
TOTAL_TARGETS = 500

########################################
# 2. Rocket Environment Class
########################################
class RocketEnv:
    def __init__(self, weight=WEIGHT, thrust=THRUST, drag_coefficient=DRAG_COEFF, tolerance=TOLERANCE):
        self.weight = weight
        self.thrust = thrust
        self.drag_coefficient = drag_coefficient
        self.tolerance = tolerance
        self.reset()

    def generate_random_target(self, radius=TARGET_RADIUS):
        while True:
            point = (2 * radius) * np.random.rand(3) - radius
            if np.linalg.norm(point) <= radius and point[0] >= 0 and point[1] >= 0 and point[2] >= 3:
                return point

    def reset(self, target=None):
        self.x, self.y, self.z = 0.0, 0.0, 0.0
        self.vx, self.vy, self.vz = 0.0, 0.0, self.thrust / self.weight
        self.target = target if target is not None else self.generate_random_target()
        self.prev_distance = float('inf')
        return self._get_state()

    def step(self, action):
        angle_x_deg, angle_y_deg = action
        angle_x_rad = math.radians(angle_x_deg)
        angle_y_rad = math.radians(angle_y_deg)

        thrust_x = self.thrust * math.sin(angle_x_rad)
        thrust_y = self.thrust * math.sin(angle_y_rad)
        thrust_z = self.thrust * math.cos(angle_x_rad) * math.cos(angle_y_rad)

        ax = thrust_x / self.weight - self.drag_coefficient * self.vx
        ay = thrust_y / self.weight - self.drag_coefficient * self.vy
        az = thrust_z / self.weight - GRAVITY - self.drag_coefficient * self.vz

        self.vx += ax * TIME_STEP
        self.vy += ay * TIME_STEP
        self.vz += az * TIME_STEP

        self.x += self.vx * TIME_STEP
        self.y += self.vy * TIME_STEP
        self.z += self.vz * TIME_STEP

        next_state = self._get_state()
        dist_to_target = np.linalg.norm([self.x - self.target[0], self.y - self.target[1], self.z - self.target[2]])

        reward = -dist_to_target
        if dist_to_target < self.prev_distance:
            reward += 0.7  # Incremental reward for moving closer
        self.prev_distance = dist_to_target

        done = self.z <= 0
        if done and dist_to_target <= self.tolerance:
            reward += 1.0

        return next_state, reward, done, {}

    def _get_state(self):
        dist_to_target = np.linalg.norm([self.x - self.target[0], self.y - self.target[1], self.z - self.target[2]])
        angle_x = math.atan2(self.target[0] - self.x, self.target[2] - self.z)
        angle_y = math.atan2(self.target[1] - self.y, self.target[2] - self.z)
        return np.array([self.x, self.y, self.z, self.vx, self.vy, self.vz,
                         self.target[0], self.target[1], self.target[2],
                         dist_to_target, angle_x, angle_y], dtype=np.float32)

########################################
# 3. Neural Network Policy
########################################
class RocketPolicy(nn.Module):
    def __init__(self):
        super(RocketPolicy, self).__init__()
        self.net = nn.Sequential(
            nn.Linear(12, 128),  # Updated input size
            nn.ReLU(),
            nn.Linear(128, 128),
            nn.ReLU(),
            nn.Linear(128, 64),
            nn.ReLU(),
            nn.Linear(64, 2)
        )

    def forward(self, state):
        return self.net(state)

########################################
# 4. Pre-Training Using Supervised Learning
########################################

# Filepaths to save the model and optimizer states
MODEL_SAVE_PATH = "rocket_policy.pth"
OPTIMIZER_SAVE_PATH = "optimizer_state.pth"

def generate_training_data(env, num_samples):
    data, labels = [], []
    for _ in range(num_samples):
        target = env.generate_random_target()
        env.reset(target)
        trajectory, servo_x_angles, servo_y_angles = calculate_trajectory_with_servos(env.thrust / env.weight, target)
        for i, (x, y, z) in enumerate(trajectory):
            dist_to_target = np.linalg.norm([x - target[0], y - target[1], z - target[2]])
            angle_x = math.atan2(target[0] - x, target[2] - z)
            angle_y = math.atan2(target[1] - y, target[2] - z)
            state = [x, y, z, env.vx, env.vy, env.vz, target[0], target[1], target[2], dist_to_target, angle_x, angle_y]
            data.append(state)
            labels.append([servo_x_angles[i], servo_y_angles[i]])
    return np.array(data), np.array(labels)


def calculate_trajectory_with_servos(initial_velocity, target_position):
    x, y, z = 0.0, 0.0, 0.0
    vx, vy, vz = 0.0, 0.0, initial_velocity

    trajectory = []
    servo_x_angles = []
    servo_y_angles = []

    while z > 0 or len(trajectory) == 0:
        trajectory.append((x, y, z))
        error_x = target_position[0] - x
        error_y = target_position[1] - y
        error_z = target_position[2] - z

        angle_x = math.atan2(error_x, error_z)
        angle_y = math.atan2(error_y, error_z)

        servo_x_angles.append(math.degrees(angle_x))
        servo_y_angles.append(math.degrees(angle_y))

        thrust_x = THRUST * math.sin(angle_x)
        thrust_y = THRUST * math.sin(angle_y)
        thrust_z = THRUST * math.cos(angle_x) * math.cos(angle_y)

        ax = thrust_x / WEIGHT - DRAG_COEFF * vx
        ay = thrust_y / WEIGHT - DRAG_COEFF * vy
        az = thrust_z / WEIGHT - GRAVITY - DRAG_COEFF * vz

        vx += ax * TIME_STEP
        vy += ay * TIME_STEP
        vz += az * TIME_STEP

        x += vx * TIME_STEP
        y += vy * TIME_STEP
        z += vz * TIME_STEP

        if len(trajectory) > 500:  # Prevent infinite loops
            break

    return trajectory, servo_x_angles, servo_y_angles


def pre_train_policy(policy, data, labels, lr=1e-3, epochs=3000):
    optimizer = optim.Adam(policy.parameters(), lr=lr)
    criterion = nn.MSELoss()
    data, labels = torch.FloatTensor(data), torch.FloatTensor(labels)
    for epoch in range(epochs):
        optimizer.zero_grad()
        predictions = policy(data)
        loss = criterion(predictions, labels)
        loss.backward()
        optimizer.step()
        print(f"Pre-Training Epoch {epoch + 1}/{epochs}, Loss: {loss.item():.4f}")

########################################
# 5. Training and Evaluation with RL
########################################
def train_policy(policy, env, training_targets, lr=LEARNING_RATE):
    # Initialize optimizer
    optimizer = optim.Adam(policy.parameters(), lr=lr)
    scheduler = optim.lr_scheduler.StepLR(optimizer, step_size=100, gamma=0.9)

    # Load optimizer state if exists
    if os.path.exists(OPTIMIZER_SAVE_PATH):
        optimizer.load_state_dict(torch.load(OPTIMIZER_SAVE_PATH))
        print("Loaded optimizer state.")

    for episode, target in enumerate(training_targets):
        state = env.reset(target)
        done = False
        log_probs = []
        rewards = []
        std_dev = max(0.5, 2.0 - (episode / 100))  # Gradually reduce exploration noise

        while not done:
            state_tensor = torch.FloatTensor(state).unsqueeze(0)
            mean_angles = policy(state_tensor)
            dist = torch.distributions.Normal(mean_angles, torch.tensor([std_dev, std_dev]))

            sampled_action = dist.sample()
            log_prob = dist.log_prob(sampled_action).sum()

            angle_x, angle_y = sampled_action.squeeze().tolist()
            angle_x, angle_y = np.clip(angle_x, -90, 90), np.clip(angle_y, -90, 90)

            next_state, reward, done, _ = env.step([angle_x, angle_y])

            log_probs.append(log_prob)
            rewards.append(reward)
            state = next_state

        total_return = sum(rewards)
        loss = -sum(log_probs) * total_return

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        scheduler.step()

        print(f"Episode {episode + 1}, Target: {target}, Total Return: {total_return:.3f}, Loss: {loss.item():.6f}")

        # Save the model and optimizer state after each episode
        torch.save(policy.state_dict(), MODEL_SAVE_PATH)
        torch.save(optimizer.state_dict(), OPTIMIZER_SAVE_PATH)

def evaluate_policy(policy, env, testing_targets):
    success_count = 0
    total_distance = 0

    for target in testing_targets:
        state = env.reset(target)
        done = False
        distances = []

        while not done:
            state_tensor = torch.FloatTensor(state).unsqueeze(0)
            with torch.no_grad():
                angles = policy(state_tensor)[0]
            angle_x, angle_y = angles.squeeze().tolist()
            angle_x, angle_y = np.clip(angle_x, -90, 90), np.clip(angle_y, -90, 90)

            state, _, done, _ = env.step([angle_x, angle_y])
            distances.append(np.linalg.norm([env.x - target[0], env.y - target[1], env.z - target[2]]))

        total_distance += np.mean(distances)
        if np.mean(distances) <= TOLERANCE:
            success_count += 1

    success_rate = (success_count / len(testing_targets)) * 100
    avg_distance = total_distance / len(testing_targets)
    print(f"Success Rate: {success_rate:.2f}%, Average Distance: {avg_distance:.3f}")

########################################
# 6. Animation Setup
########################################
def calculate_trajectory_with_servos_ml(initial_velocity, target_position, policy):
    x, y, z = 0.0, 0.0, 0.0
    vx, vy, vz = 0.0, 0.0, initial_velocity

    trajectory = []
    servo_x_angles = []
    servo_y_angles = []

    while z > 0 or len(trajectory) == 0:
        trajectory.append((x, y, z))
        state = np.array([x, y, z, vx, vy, vz, target_position[0], target_position[1], target_position[2],
                          np.linalg.norm([x - target_position[0], y - target_position[1], z - target_position[2]]),
                          math.atan2(target_position[0] - x, target_position[2] - z),
                          math.atan2(target_position[1] - y, target_position[2] - z)], dtype=np.float32)
        state_tensor = torch.FloatTensor(state).unsqueeze(0)

        with torch.no_grad():
            angles = policy(state_tensor)[0]

        angle_x_deg = np.clip(angles[0].item(), -90, 90)
        angle_y_deg = np.clip(angles[1].item(), -90, 90)

        servo_x_angles.append(angle_x_deg)
        servo_y_angles.append(angle_y_deg)

        ax_rad, ay_rad = math.radians(angle_x_deg), math.radians(angle_y_deg)

        thrust_x = THRUST * math.sin(ax_rad)
        thrust_y = THRUST * math.sin(ay_rad)
        thrust_z = THRUST * math.cos(ax_rad) * math.cos(ay_rad)

        ax = thrust_x / WEIGHT - DRAG_COEFF * vx
        ay = thrust_y / WEIGHT - DRAG_COEFF * vy
        az = thrust_z / WEIGHT - GRAVITY - DRAG_COEFF * vz

        vx += ax * TIME_STEP
        vy += ay * TIME_STEP
        vz += az * TIME_STEP

        x += vx * TIME_STEP
        y += vy * TIME_STEP
        z += vz * TIME_STEP

        if len(trajectory) > 500:
            break

    return trajectory, servo_x_angles, servo_y_angles

def generate_target_points(env, num_points):
    return [env.generate_random_target() for _ in range(num_points)]

 
def visualize_trained_model(policy, env, num_points=100):
    """
    Visualize the trained model's performance on random target points.

    Args:
        policy: The trained RocketPolicy model.
        env: The RocketEnv environment instance.
        num_points: Number of random target points to visualize.
    """
    fig = plt.figure(figsize=(12, 10))
    ax1 = fig.add_subplot(2, 1, 1, projection='3d')
    ax2 = fig.add_subplot(2, 1, 2)

    ax1.set_title("3D Trajectory Visualization")
    ax1.set_xlim(-TARGET_RADIUS, TARGET_RADIUS)
    ax1.set_ylim(-TARGET_RADIUS, TARGET_RADIUS)
    ax1.set_zlim(0, TARGET_RADIUS)
    ax1.set_xlabel("X Position (m)")
    ax1.set_ylabel("Y Position (m)")
    ax1.set_zlabel("Z Position (m)")

    ax2.set_title("Distance to Target")
    ax2.set_xlim(0, num_points)
    ax2.set_ylim(0, TARGET_RADIUS)
    ax2.set_xlabel("Target Index")
    ax2.set_ylabel("Distance (m)")
    ax2.grid(True)

    distances_to_target = []

    for i in range(num_points):
        target = env.generate_random_target()
        env.reset(target)

        trajectory, servo_x_angles, servo_y_angles = calculate_trajectory_with_servos_ml(
            THRUST / WEIGHT, target, policy
        )

        # Final point distance
        final_point = trajectory[-1]
        final_distance = np.linalg.norm([
            final_point[0] - target[0],
            final_point[1] - target[1],
            final_point[2] - target[2]
        ])
        distances_to_target.append(final_distance)

        # Plot trajectory
        trajectory_x = [pos[0] for pos in trajectory]
        trajectory_y = [pos[1] for pos in trajectory]
        trajectory_z = [pos[2] for pos in trajectory]

        ax1.plot(trajectory_x, trajectory_y, trajectory_z, label=f"Target {i+1}")
        ax1.scatter(target[0], target[1], target[2], c='red', s=50, label=f"Target {i+1} (End)")

    # Plot distances
    ax2.plot(range(num_points), distances_to_target, marker='o', color='green')

    
    plt.tight_layout()
    plt.show()


########################################
# 7. Main Function
if __name__ == "__main__":
    env = RocketEnv()
    policy = RocketPolicy()

    # Check if a saved model exists, and load it if so
    if os.path.exists(MODEL_SAVE_PATH):
        try:
            policy.load_state_dict(torch.load(MODEL_SAVE_PATH))
            print("Loaded saved policy model.")
        except RuntimeError as e:
            print("Error loading model:", e)
            print("Retraining with new architecture.")
            os.remove(MODEL_SAVE_PATH)

    # Pre-Training
    if not os.path.exists(MODEL_SAVE_PATH):  # Only pre-train if no saved model exists
        print("=== Pre-Training Policy ===")
        data, labels = generate_training_data(env, num_samples=1000)
        pre_train_policy(policy, data, labels)

    # Pre-Training
    print("=== Pre-Training Policy ===")
    data, labels = generate_training_data(env, num_samples=1000)
    pre_train_policy(policy, data, labels)

    # Generate 500 target points
    all_targets = generate_target_points(env, TOTAL_TARGETS)

    # Split into training (75%) and testing (25%)
    train_targets = all_targets[:int(0.75 * TOTAL_TARGETS)]
    test_targets = all_targets[int(0.25 * TOTAL_TARGETS):]

    print("=== Training Policy ===")
    train_policy(policy, env, train_targets, lr=LEARNING_RATE)
    print("=== Training Complete ===")

    print("=== Evaluating Policy ===")
    evaluate_policy(policy, env, test_targets)
    print("=== Evaluation Complete ===")

    # Visualize the trained model's performance on random targets
    print("=== Visualizing Trained Model ===")
    visualize_trained_model(policy, env, num_points=10)

    # Animation setup with test points
    fig = plt.figure(figsize=(12, 10))

    ax1 = fig.add_subplot(3, 1, 1)
    line1, = ax1.plot([], [], color='orange', label='Servo X Angle')
    line2, = ax1.plot([], [], color='purple', label='Servo Y Angle')
    ax1.set_xlim(0, 100)
    ax1.set_ylim(-90, 90)
    ax1.set_xlabel('Time Step')
    ax1.set_ylabel('Servo Angle (degrees)')
    ax1.legend()
    ax1.grid(True)

    ax2 = fig.add_subplot(3, 1, 2, projection='3d')
    trajectory_line, = ax2.plot([], [], [], color='blue', label='Trajectory')
    target_scatter = ax2.scatter([], [], [], color='red', label='Target', s=50)
    ax2.set_xlim(-TARGET_RADIUS, TARGET_RADIUS)
    ax2.set_ylim(-TARGET_RADIUS, TARGET_RADIUS)
    ax2.set_zlim(0, TARGET_RADIUS)
    ax2.set_xlabel('X Position (m)')
    ax2.set_ylabel('Y Position (m)')
    ax2.set_zlabel('Z Position (m)')
    ax2.legend()

    ax3 = fig.add_subplot(3, 1, 3)
    distance_line, = ax3.plot([], [], color='green', label='Distance to Target')
    ax3.set_xlim(0, 600)
    ax3.set_ylim(0, TARGET_RADIUS)
    ax3.set_xlabel('Target Number')
    ax3.set_ylabel('Distance (m)')
    ax3.legend()
    ax3.grid(True)

    num_targets = len(test_targets)
    current_target = 0
    servo_x_data = []
    servo_y_data = []
    trajectory_x = []
    trajectory_y = []
    trajectory_z = []
    distances_to_target = []

    def init():
        line1.set_data([], [])
        line2.set_data([], [])
        trajectory_line.set_data([], [])
        trajectory_line.set_3d_properties([])
        target_scatter._offsets3d = ([], [], [])
        distance_line.set_data([], [])
        return line1, line2, trajectory_line, target_scatter, distance_line

    def update(frame):
     global current_target, servo_x_data, servo_y_data, trajectory_x, trajectory_y, trajectory_z, distances_to_target

     if current_target >= num_targets:
        return line1, line2, trajectory_line, target_scatter, distance_line

     target_point = test_targets[current_target]
     initial_velocity = THRUST / WEIGHT

    # Calculate the trajectory and angles
     trajectory, servo_x_angles, servo_y_angles = calculate_trajectory_with_servos_ml(
        initial_velocity, target_point, policy
     )

    # Calculate the distance from the final point of the trajectory to the target
     final_point = trajectory[-1]
     final_distance_to_target = np.linalg.norm(
        [final_point[0] - target_point[0],
         final_point[1] - target_point[1],
         final_point[2] - target_point[2]]
     )
     distances_to_target.append(final_distance_to_target)

     servo_x_data = servo_x_angles
     servo_y_data = servo_y_angles
     time_steps = range(len(servo_x_data))

     trajectory_x = [pos[0] for pos in trajectory]
     trajectory_y = [pos[1] for pos in trajectory]
     trajectory_z = [pos[2] for pos in trajectory]

     line1.set_data(time_steps, servo_x_data)
     line2.set_data(time_steps, servo_y_data)
     trajectory_line.set_data(trajectory_x, trajectory_y)
     trajectory_line.set_3d_properties(trajectory_z)
     target_scatter._offsets3d = ([target_point[0]], [target_point[1]], [target_point[2]])
     distance_line.set_data(range(len(distances_to_target)), distances_to_target)

     current_target += 1

    # Ensure the distance plot remains intact after all frames are processed
     if current_target >= num_targets:
        plt.draw()

     return line1, line2, trajectory_line, target_scatter, distance_line


    ani = FuncAnimation(
        fig, update, frames=range(num_targets),
        init_func=init, blit=False, interval=500
    )

    plt.tight_layout()
    plt.show()

