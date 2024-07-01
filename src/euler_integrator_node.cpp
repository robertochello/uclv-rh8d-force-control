#include "rclcpp/rclcpp.hpp"
#include "rclcpp/wait_for_message.hpp"
#include "uclv_seed_robotics_ros_interfaces/msg/motor_positions.hpp"
#include "uclv_seed_robotics_ros_interfaces/msg/velocity_array.hpp" // Custom message type
#include "std_srvs/srv/set_bool.hpp"
#include "std_msgs/msg/int32.hpp"
#include <cmath>
#include <vector>

class EulerIntegrator : public rclcpp::Node
{
public:
    double dt_;
    bool initial_condition_received_;
    double time_;
    uclv_seed_robotics_ros_interfaces::msg::MotorPositions motor_positions_;
    std::vector<float> velocities_;
    rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr start_stop_service_;
    rclcpp::Subscription<uclv_seed_robotics_ros_interfaces::msg::VelocityArray>::SharedPtr velocity_value_sub_;
    rclcpp::Publisher<uclv_seed_robotics_ros_interfaces::msg::MotorPositions>::SharedPtr desired_position_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    EulerIntegrator()
        : Node("euler_integrator"),
          initial_condition_received_(false),
          time_(0.0)
    {
        dt_ = this->declare_parameter<double>("dt", 0.1);

        start_stop_service_ = create_service<std_srvs::srv::SetBool>(
            "startstop", std::bind(&EulerIntegrator::service_callback, this,
                                   std::placeholders::_1, std::placeholders::_2));

        velocity_value_sub_ = this->create_subscription<uclv_seed_robotics_ros_interfaces::msg::VelocityArray>(
            "/cmd/velocity_value", 10,
            std::bind(&EulerIntegrator::velocity_callback, this, std::placeholders::_1));

        desired_position_pub_ = this->create_publisher<uclv_seed_robotics_ros_interfaces::msg::MotorPositions>(
            "desired_position", 10);

        timer_ = this->create_wall_timer(
            std::chrono::duration<double>(dt_),
            std::bind(&EulerIntegrator::integrate, this));

        timer_->cancel(); // Initially, the timer is stopped
    }

private:
    double sin_fun_(double t)
    {
        double OFF = 2000.0;
        double AMP = 1000.0;
        double F = 1.0;
        return ((AMP * std::sin(2.0 * M_PI * F * t)) + OFF);
    }

    void integrate()
    {
        if (!initial_condition_received_)
        {
            RCLCPP_WARN(this->get_logger(), "Waiting for initial motor positions...");
            return;
        }

        // Increment time
        time_ += dt_;

        // Apply Euler integration at each time step
        for (size_t i = 0; i < motor_positions_.positions.size(); i++)
        {
            float V = (i < velocities_.size()) ? velocities_[i] : sin_fun_(time_);
            motor_positions_.positions[i] = motor_positions_.positions[i] + dt_ * V;
        }

        // Publish the new positions
        desired_position_pub_->publish(motor_positions_);
    }

    void velocity_callback(const uclv_seed_robotics_ros_interfaces::msg::VelocityArray::SharedPtr msg)
    {
        velocities_.clear();
        for (const auto &velocity : msg->velocities)
        {
            velocities_.push_back(velocity.data);
        }
    }

    void service_callback(const std::shared_ptr<std_srvs::srv::SetBool::Request> request,
                          std::shared_ptr<std_srvs::srv::SetBool::Response> response)
    {
        if (request->data)
        {
            if (timer_->is_canceled())
            {
                // Wait for the initial condition
                if (rclcpp::wait_for_message<uclv_seed_robotics_ros_interfaces::msg::MotorPositions>(
                        motor_positions_, this->shared_from_this(), "/motor_state", std::chrono::seconds(1)))
                {
                    initial_condition_received_ = true;
                    velocities_.resize(motor_positions_.positions.size(), 0.0);

                    // Log the initial conditions
                    for (size_t i = 0; i < motor_positions_.positions.size(); i++)
                    {
                        RCLCPP_INFO(this->get_logger(), "ID: %d, Initial Position: %f", motor_positions_.ids[i], motor_positions_.positions[i]);
                    }

                    timer_->reset();
                    response->success = true;
                    response->message = "Timer started.";
                    RCLCPP_INFO(this->get_logger(), "Timer started.");
                }
                else
                {
                    response->success = false;
                    response->message = "Failed to receive initial motor positions.";
                    RCLCPP_WARN(this->get_logger(), "Failed to receive initial motor positions.");
                }
            }
            else
            {
                response->success = false;
                response->message = "Timer is already running.";
                RCLCPP_WARN(this->get_logger(), "Timer is already running.");
            }
        }
        else
        {
            if (!timer_->is_canceled())
            {
                timer_->cancel();
                response->success = true;
                response->message = "Timer stopped.";
                RCLCPP_INFO(this->get_logger(), "Timer stopped.");
            }
            else
            {
                response->success = true;
                response->message = "Timer was not running.";
                RCLCPP_WARN(this->get_logger(), "Timer is not running.");
            }
        }
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    try
    {
        auto euler_integrator_node = std::make_shared<EulerIntegrator>();
        rclcpp::spin(euler_integrator_node);
    }
    catch (const std::exception &e)
    {
        RCLCPP_FATAL(rclcpp::get_logger("rclcpp"), "Exception caught: %s", e.what());
        rclcpp::shutdown();
        return 1;
    }

    rclcpp::shutdown();
    return 0;
}
