from typing import TypedDict

class LocationType(TypedDict):
    id: str
    latitude: float | None
    longitude: float | None
    altitude: float | None
    hdop: float | None
    satellites: int | None
    date_time: str | None

class AddressType(TypedDict):
    id: str
    country: str | None
    country_code: str | None
    state: str | None
    region: str | None
    city: str | None
    postcode: str | None
    road: str | None
    house_number: str | None