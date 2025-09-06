# PUB/SUB Event Endpoint & Topics

## 1. 개요
- **Endpoint**: `ipc:///tmp/app.events`
- **Protocol**: ZeroMQ PUB/SUB
- **Publisher**: Vision Backend
- **Subscribers**: Vision Frontend

---

## 2. 메시지 구조 (토픽별 정의)

### Topic: `det` (`kTopicDetections`)
- **설명**: 객체 검출 결과 전송
- **Payload 형식 (JSON)**:
```json
{
  "object": "car",
  "confidence": 0.92,
  "bbox": [100, 120, 200, 240]
}

```
### Topic: `blt` (`kTopicBluetooth`)
- **설명**: 블루투스 검색 목록
- **Payload 형식 (JSON)**:
```json
{
  "object": "car",
  "confidence": 0.92,
  "bbox": [100, 120, 200, 240]
}

```
