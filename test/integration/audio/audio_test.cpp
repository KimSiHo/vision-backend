#include <gtest/gtest.h>
#include <pulse/pulseaudio.h>

#include <cstdlib>
#include <iostream>
#include <typeinfo>

#include "services/audio/audio_service.hpp"

TEST(AudioServiceIntegration, CanConnectAndQueryVolume) {
  //  SCOPED_TRACE("🔊 Checking PulseAudio volume query");
  // TEST(AudioServiceIntegration, CanConnectAndQueryVolume) {

  // #define DISPLAY(desc) std::cerr << "[INFO] " << desc << std::endl;

  // TEST(AudioServiceIntegration, CanConnectAndQueryVolume) {
  //   DISPLAY("🔊 Connects to PulseAudio and verifies volume range 0–100%

  AudioService service;
  std::cerr << "AudioService constructed ✅" << std::endl;
  std::cout << "Sink name: " << service.getSink() << std::endl;
  auto vol = service.getVolume();
  std::cout << "Type: " << typeid(vol).name() << std::endl;

  if (!vol.has_value()) {
    GTEST_SKIP() << "Failed to get volume — PulseAudio context unavailable.";
  }

  EXPECT_GE(*vol, 0);
  EXPECT_LE(*vol, 100);
  std::cout << "Current volume: " << *vol << "%" << std::endl;
}
