#include <zephyr/kernel.h>
  #include <zephyr/logging/log.h>
  #include <zephyr/device.h>
  #include <zephyr/drivers/pwm.h>

  LOG_MODULE_REGISTER(app_pwm, LOG_LEVEL_INF);

  /* PWM Configuration for 1kHz */
  #define PWM_PERIOD_NS       PWM_MSEC(1)              // 1ms period = 1kHz
  #define STEP_DELAY_MS       20                       // 20ms between steps
  #define NUM_STEPS           100                      // 100 steps (0-100%)

  #define PWM_NODE            DT_ALIAS(pwm_led0)
  static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(PWM_NODE);

  int main(void)
  {
        uint32_t pulse_width;
        uint32_t step_size;
        int ret;

        LOG_INFO("PWM Demo: 0%% to 100%% duty cycle at 1kHz");

        /* Check if PWM device is ready */
        if (!pwm_is_ready_dt(&pwm_led)) {
                LOG_INFO("Error: PWM device is not ready");
                return -1;
        }

        LOG_INFO("PWM ready on channel %d\n", pwm_led.channel);

        /* Calculate step size: 1,000,000 ns / 100 steps = 10,000 ns per step */
        step_size = PWM_PERIOD_NS / NUM_STEPS;

        while (1) {
                /* Ramp UP: 0% to 100% duty cycle */
                LOG_INFO("\n--- Ramping UP ---\n");
                for (int i = 0; i <= NUM_STEPS; i++) {
                        pulse_width = i * step_size;

                        ret = pwm_set_dt(&pwm_led, PWM_PERIOD_NS, pulse_width);
                        if (ret) {
                                LOG_INFO("Error: Failed to set PWM\n");
                                return -1;
                        }

                        LOG_INFO("Duty: %3d%% | Pulse: %7u ns\n", i, pulse_width);
                        k_msleep(STEP_DELAY_MS);
                }

                k_sleep(K_MSEC(500));  // Pause at 100%

                /* Ramp DOWN: 100% to 0% duty cycle */
                LOG_INFO("\n--- Ramping DOWN ---\n");
                for (int i = NUM_STEPS; i >= 0; i--) {
                        pulse_width = i * step_size;

                        ret = pwm_set_dt(&pwm_led, PWM_PERIOD_NS, pulse_width);
                        if (ret) {
                                LOG_INFO("Error: Failed to set PWM\n");
                                return -1;
                        }

                        LOG_INFO("Duty: %3d%% | Pulse: %7u ns\n", i, pulse_width);
                        k_msleep(STEP_DELAY_MS);
                }

                k_sleep(K_MSEC(500));  // Pause at 0%
        }

        return 0;
  }