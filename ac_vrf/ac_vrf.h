#pragma once
#include "esphome.h"

namespace ac_vrf {

// 噪音模式枚举
enum NoiseMode { DAY, NIGHT };

class ACVRF : public PollingComponent, public ModbusController {
 public:
  // 构造函数：传入设备地址和分区数量
  ACVRF(uint8_t address, uint8_t zones) : address_(address), zones_(zones) {}

  void setup() override {
    PollingComponent::setup();
    ModbusController::setup();
    this->set_address(address_);
    this->set_update_interval(5000);
    ESP_LOGCONFIG(TAG, "ACVRF 初始化: 地址=%u, 分区=%u", address_, zones_);

    // 注册每区实体
    for (uint8_t i = 0; i < zones_; i++) {
      // 温度
      auto *temp = new Sensor();
      temp->set_name("风盘" + String(i + 1) + " 温度");
      register_sensor(temp);
      temps_.push_back(temp);
      // 运行模式
      auto *mode = new Sensor();
      mode->set_name("风盘" + String(i + 1) + " 运行模式");
      register_sensor(mode);
      modes_.push_back(mode);
      // 风速
      auto *fan = new Sensor();
      fan->set_name("风盘" + String(i + 1) + " 风速");
      register_sensor(fan);
      fans_.push_back(fan);
      // 开关状态
      auto *sw = new BinarySensor();
      sw->set_name("风盘" + String(i + 1) + " 开关状态");
      register_binary_sensor(sw);
      switches_.push_back(sw);
      // VIP模式
      auto *vip = new BinarySensor();
      vip->set_name("风盘" + String(i + 1) + " VIP模式");
      register_binary_sensor(vip);
      vips_.push_back(vip);
      // 网络状态
      auto *net = new BinarySensor();
      net->set_name("风盘" + String(i + 1) + " 网络状态异常");
      register_binary_sensor(net);
      nets_.push_back(net);
      // 湿度
      auto *hum = new Sensor();
      hum->set_name("风盘" + String(i + 1) + " 湿度");
      register_sensor(hum);
      hums_.push_back(hum);
    }
  }

  void update() override {
    PollingComponent::update();
    // 每区读取11个Input寄存器，从协议地址6000起
    for (uint8_t i = 0; i < zones_; i++) {
      uint16_t proto_reg = 6000 + i * 20;
      this->read_input_registers(proto_reg, 11, [this, i](const std::vector<uint16_t> &v) {
        // v[0..2]三组故障位，不拆分具体位，可根据需要扩展为多个binary_sensor
        // v[3] 环境温度*10
        temps_[i]->publish_state(int16_t(v[3]) * 0.1f);
        // v[4] 运行模式
        modes_[i]->publish_state(v[4]);
        // v[5] 风速
        fans_[i]->publish_state(v[5]);
        // v[6] VIP(<<1) and 开关(bit0)
        switches_[i]->publish_state(v[6] & 0x01);
        vips_[i]->publish_state((v[6] >> 1) & 0x01);
        // v[7] 网络状态(bit0)
        nets_[i]->publish_state(v[7] & 0x01);
        // v[8] 湿度
        hums_[i]->publish_state(v[8]);
        // 更多位字段(v[9], v[10])可根据需求解析
      });
    }
  }

  void set_noise_mode(NoiseMode mode) {
    // 协议写寄存器0x0000 + (40024-40001)=23
    this->write_single_register(23, mode == NIGHT ? 1 : 2);
    ESP_LOGD(TAG, "设置噪音模式: %s", mode == NIGHT ? "夜间" : "日间");
  }

  void set_system_on(bool on) {
    // 协议写寄存器 40001->0
    this->write_single_register(0, on ? 1 : 0);
    ESP_LOGD(TAG, "系统总开关: %s", on ? "开启" : "关闭");
  }

 private:
  static const char *TAG;
  uint8_t address_, zones_;
  std::vector<Sensor *> temps_, modes_, fans_, hums_;
  std::vector<BinarySensor *> switches_, vips_, nets_;
};

}  // namespace ac_vrf