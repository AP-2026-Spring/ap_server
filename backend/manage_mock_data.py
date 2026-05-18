import sqlite3
import sys
import os

DB_PATH = "detections.db"

def init_db():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS detections (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            camera_id INTEGER,
            class_name TEXT NOT NULL,
            confidence REAL NOT NULL,
            x REAL,
            y REAL,
            w REAL,
            h REAL,
            timestamp TEXT NOT NULL,
            image_data TEXT
        );
    ''')
    conn.commit()
    return conn

def insert_mock_data():
    conn = init_db()
    cursor = conn.cursor()
    
    # Check if data already exists to prevent duplicate insertion
    cursor.execute("SELECT COUNT(*) FROM detections")
    if cursor.fetchone()[0] > 0:
        print("[Info] Mock data already exists in the database. Clear it first if you want to re-insert.")
        conn.close()
        return

    mock_data = [
        (101, "mouse", 0.92, 120.5, 85.0, 40.0, 25.0, "2026-05-18 14:15:22", "snapshot.png"),
        (101, "mouse", 0.88, 150.0, 90.0, 38.0, 22.0, "2026-05-18 10:05:11", ""),
        (101, "cockroach", 0.75, 200.0, 200.0, 10.0, 10.0, "2026-05-17 22:30:00", "")
    ]
    
    cursor.executemany('''
        INSERT INTO detections (camera_id, class_name, confidence, x, y, w, h, timestamp, image_data)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    ''', mock_data)
    
    conn.commit()
    conn.close()
    print("[Success] Mock data has been inserted into detections.db")

def clear_mock_data():
    if not os.path.exists(DB_PATH):
        print("[Info] Database file does not exist.")
        return

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute("DELETE FROM detections")
    conn.commit()
    conn.close()
    print("[Success] All mock data has been deleted from detections.db")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python manage_mock_data.py [insert|clear]")
        sys.exit(1)

    command = sys.argv[1].lower()
    
    if command == "insert":
        insert_mock_data()
    elif command == "clear":
        clear_mock_data()
    else:
        print(f"Unknown command: {command}")
        print("Usage: python manage_mock_data.py [insert|clear]")
