from fastapi import HTTPException, FastAPI, status
from fastapi.responses import FileResponse
from pydantic import BaseModel 
from typing import Optional 
import os
import httpx

app = FastAPI()
DB_URL = "http://backend_database:8001" #backend_database is service name in docker-compose


class PostDataModel(BaseModel): 
    mode: Optional[str] = None
    id: str
    name: str 
    feature_vector: list[list[int]]

class FeatureEntry(BaseModel):
    name: str 
    feature_vector: list[list[int]]


@app.post("/", status_code=status.HTTP_201_CREATED)
async def post_data(data: PostDataModel): 
    feature = FeatureEntry(name=data.name, feature_vector=data.feature_vector)

    async with httpx.AsyncClient() as client:
        get_resp = await client.get(f"{DB_URL}/retrieve/{data.name}")
        if data.mode == "register": 
            if get_resp.status_code == 200:
                raise HTTPException(status_code=status.HTTP_409_CONFLICT, detail="User is already registered")
            elif get_resp.status_code == 404: 
                post_resp = await client.post(f"{DB_URL}/store", json=feature.dict())
                if post_resp.status_code == 200:
                    raise HTTPException(status_code=status.HTTP_200_OK, detail="User is registered successfully")
                else: 
                    raise HTTPException(status_code=post_resp.status_code)
            else: 
                try: 
                    get_resp.raise_for_status()
                except httpx.HTTPStatusError as e: 
                    raise HTTPException(status_code=e.get_resp.status_code, detail=e.get_resp.json())
                except httpx.RequestError as e:
                    raise HTTPException(status_code=500, detail=f"Network error: {str(e)}")

        else:
            if get_resp.status_code == 404:
                raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="User is not registered")
            else: 
                with open('./ios_ppfr/put.txt', 'w') as f: 
                    f.write(f"{data.id}\n")
                    f.write(f"{data.name}\n")
                    f.write(str(data.feature_vector))

                raise HTTPException(status_code=status.HTTP_200_OK, detail="Post request is successful")


@app.get("/ios_ppfr/{filename:path}", status_code=status.HTTP_200_OK)
#file_path is the name of path parameter, access as a str in func
#:path is type converter, telling FastAPI to accept normally-reserved chars
async def get_result(filename: str): 
    file_path = os.path.join("./ios_ppfr", filename)

    if not os.path.exists(file_path):
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Result is not available")

    return FileResponse(path=file_path, media_type='application/json')


