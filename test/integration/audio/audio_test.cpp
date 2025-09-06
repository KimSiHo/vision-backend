#include <gtest/gtest.h>
#include <pulse/pulseaudio.h>

#include <cstdlib>
#include <iostream>
#include <typeinfo>

#include "services/audio/audio_service.hpp"

TEST(AudioServiceIntegration, CanConnectAndQueryVolume) {
  // const char* xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");
  // if (xdg_runtime_dir)
  //   std::cout << "XDG_RUNTIME_DIR=" << xdg_runtime_dir << std::endl;
  // else
  //   std::cout << "XDG_RUNTIME_DIR not set" << std::endl;
  //
  //// ----------------------------------------
  //// PulseAudio 직접 연결 테스트
  //// ----------------------------------------
  // pa_mainloop* ml = pa_mainloop_new();
  // ASSERT_NE(ml, nullptr) << "Failed to create mainloop";
  //
  // pa_mainloop_api* api = pa_mainloop_get_api(ml);
  // pa_context* ctx = pa_context_new(api, "AudioServiceTest");
  // ASSERT_NE(ctx, nullptr) << "Failed to create context";
  //
  // int ret = pa_context_connect(ctx, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
  // if (ret < 0) {
  //   std::cerr << "pa_context_connect() failed: " << pa_strerror(pa_context_errno(ctx)) << std::endl;
  //   pa_context_unref(ctx);
  //   pa_mainloop_free(ml);
  //   GTEST_SKIP() << "PulseAudio connection failed — skipping test.";
  // }
  //
  //// Wait until context is ready
  // while (true) {
  //   pa_mainloop_iterate(ml, 1, nullptr);
  //   auto state = pa_context_get_state(ctx);
  //   if (state == PA_CONTEXT_READY) break;
  //   if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
  //     std::cerr << "Context connection failed: " << pa_strerror(pa_context_errno(ctx)) << std::endl;
  //     pa_context_disconnect(ctx);
  //     pa_context_unref(ctx);
  //     pa_mainloop_free(ml);
  //     GTEST_SKIP() << "PulseAudio context not ready — skipping test.";
  //   }
  // }
  //
  // std::cerr << "PulseAudio connected successfully ✅" << std::endl;

  // ----------------------------------------
  // 실제 서비스 로직 테스트
  // ----------------------------------------
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

  // Cleanup
  //  pa_context_disconnect(ctx);
  //  pa_context_unref(ctx);
  //  pa_mainloop_free(ml);
}
