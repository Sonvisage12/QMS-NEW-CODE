#include "SharedQueue1.h"

SharedQueue::SharedQueue(const String& ns) : namespaceStr(ns), counter(1) {}

void SharedQueue::load() {
  prefs.begin(namespaceStr.c_str(), false);
  counter = prefs.getInt("counter", 1);
  int count = prefs.getInt("count", 0);
  queue.clear();

  for (int i = 0; i < count; i++) {
    String key = "UID_" + String(i);
    String uid = prefs.getString((key + "_uid").c_str(), "");
    int number = prefs.getInt((key + "_num").c_str(), 0);
    String timestamp = prefs.getString((key + "_time").c_str(), "");
    if (uid != "") {
      queue.push_back({ uid, timestamp, number });
    }
  }

  //sortQueue();
  prefs.end();
}


void SharedQueue::save() {
  prefs.begin(namespaceStr.c_str(), false);
  prefs.putInt("count", queue.size());
  prefs.putInt("counter", counter);

  for (size_t i = 0; i < queue.size(); i++) {
    String key = "UID_" + String(i);
    prefs.putString((key + "_uid").c_str(), queue[i].uid);
    prefs.putInt((key + "_num").c_str(), queue[i].number);
    prefs.putString((key + "_time").c_str(), queue[i].timestamp);
  }

  prefs.end();
}

// void SharedQueue::insertAt(size_t index, const String& uid, 
//                             const String& timestamp, int number) {
//     QueueEntry entry;
//     entry.uid = uid;
//     entry.timestamp = timestamp;
//     entry.number = number;
//     if (index >= entries.size()) {
//         // If index is beyond the current size, just push to the back
//         entries.push_back(entry);
//     } else {
//         // Insert at the specified position
//         entries.insert(entries.begin() + index, entry);
//     }
//     // (Optional) If your SharedQueue persists data (e.g., in Preferences),
//    save(); // you may want to save/update the storage here as well.
// }


void SharedQueue::insertAt(size_t index, const String& uid, const String& timestamp, int number) {
    QueueEntry entry = {uid, timestamp, number};

    if (index >= queue.size()) {
        queue.push_back(entry);
    } else {
        queue.insert(queue.begin() + index, entry);
    }

    save();
}

void SharedQueue::clear() {
    queue.clear();  // 🔥 Clear the internal queue
    Serial.println("🧹 SharedQueue cleared.");
    prefs.begin(namespaceStr.c_str(), false);
    prefs.clear();  // Clears all keys in the namespace
    prefs.end();
    Serial.println("🗑️ Preferences data cleared.");

    // Optionally reset counter
    counter = 1;
}

void SharedQueue::print() {
  Serial.println("\n📋 Current Patient Queue:");
  for (const auto &entry : queue) {
    Serial.printf("No: %d | UID: %s | Time: %s\n",
      entry.number, entry.uid.c_str(), entry.timestamp.c_str());
  }
  Serial.println("------------------------------");
}

void SharedQueue::add(const String& uid, const String& timestamp, int number) {
  queue.push_back({ uid, timestamp, number });
 // sortQueue();
  save();
}
void SharedQueue::add1(const String& uid, const String& timestamp, int number) {
  queue.push_back({ uid, timestamp, number });
  //sortQueue();
  save();
}
void SharedQueue::addIfNew(const String& uid, const String& timestamp, int number) {
  if (!exists(uid)) {
    add(uid, timestamp, number);
  }
}

void SharedQueue::removeByUID(const String& uid) {
  queue.erase(std::remove_if(queue.begin(), queue.end(),
    [uid](const QueueEntry& entry) { return entry.uid == uid; }),
    queue.end());
  save();
  Serial.print("\n✅ Removed from Queue: ");
  Serial.println(uid);
}

bool SharedQueue::exists(const String& uid) {
  return std::any_of(queue.begin(), queue.end(),
    [uid](const QueueEntry& entry) { return entry.uid == uid; });
}

int SharedQueue::getOrAssignPermanentNumber(const String& uid, const DateTime& now) {
  prefs.begin(namespaceStr.c_str(), false);
  int savedCount = prefs.getInt("count", 0);
  for (int i = 0; i < savedCount; i++) {
    String key = "UID_" + String(i);
    String savedUID = prefs.getString((key + "_uid").c_str(), "");
    if (savedUID == uid) {
      int num = prefs.getInt((key + "_num").c_str(), 0);
      prefs.end();
      return num;
    }
  }

  int pid = counter++;
  String key = "UID_" + String(savedCount);
  prefs.putString((key + "_uid").c_str(), uid);
  prefs.putInt((key + "_num").c_str(), pid);
  char buffer[25];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  prefs.putString((key + "_time").c_str(), String(buffer));
  prefs.putInt("count", savedCount + 1);
  prefs.putInt("counter", counter);
  prefs.end();
  return pid;
}

std::vector<QueueEntry>& SharedQueue::getQueue() {
  return queue;
}

QueueEntry SharedQueue::getEntry(String uid) {
  for (auto& entry : queue) {  // 🔥 Corrected to use queue
    if (entry.uid == uid) return entry;
  }
  return {"", "", -1};
}

// void SharedQueue::sortQueue() {
//   std::sort(queue.begin(), queue.end(), [](const QueueEntry& a, const QueueEntry& b) {
//     return a.timestamp < b.timestamp;
//   });
// }
