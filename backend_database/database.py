from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import sqlite3
import json

app = FastAPI()

DB_PATH = "features.db"

# Initialize database
def init_db():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute('''CREATE TABLE IF NOT EXISTS features (
                      name TEXT PRIMARY KEY,
                      feature_vector TEXT)''')
    conn.commit()
    conn.close()

init_db()

class FeatureEntry(BaseModel):
    name: str
    feature_vector: list[list[int]]

@app.post("/store")
def store_weight(feature: FeatureEntry):
    feature_json = json.dumps(feature.feature_vector)  #serialize to json
    try: 
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()
        cursor.execute("INSERT OR REPLACE INTO features (name, feature_vector) VALUES (?, ?)", 
                (feature.name, feature_json))
        conn.commit()
        return {"message": f"Feature of user {feature.name} stored successfully"}
    finally: 
        conn.close()

@app.get("/retrieve/{name}")
def retrieve_weight(name: str):
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute("SELECT feature_vector FROM features WHERE name=?", (name,))
    result = cursor.fetchone()
    conn.close()
    print(result)

    if result: 
        try: 
            feature = json.loads(result[0]) #deserialize json back
        except Exception as e:
            raise HTTPException(status_code=500, detail=f"Error parsing vector: {e}")
        return {"name": name, "feature_vector": feature}
    else: 
        raise HTTPException(status_code=404, detail="Name not found")

@app.delete("/delete/{name}")
def delete_weight(name: str):
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute("DELETE FROM features WHERE name=?", (name,))
    conn.commit()
    rows_deleted = cursor.rowcount
    conn.close()
    if rows_deleted == 0: 
        raise HTTPException(status_code=404, detail="Name not found")
    return {"message": f"Feature of user '{name}' deleted successfully"}


