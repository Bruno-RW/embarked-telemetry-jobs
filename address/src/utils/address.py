from time import sleep

from geopy.geocoders import Nominatim
from geopy.extra.rate_limiter import RateLimiter
from geopy.exc import GeocoderTimedOut, GeocoderServiceError

from _types import LocationType, AddressType

def findAddress(location: LocationType) -> AddressType | None:
    locator = Nominatim(user_agent="view_reverse_geocoder_example")
    rateLimiter = RateLimiter(locator.reverse, min_delay_seconds=1/20, max_retries=2)

    if location["latitude"] is None or location["longitude"] is None: return None

    latitude = location["latitude"]
    longitude = location["longitude"]

    try:
        geoLocation = rateLimiter((latitude, longitude), exactly_one=True, language="en")

    except (GeocoderTimedOut, GeocoderServiceError, Exception):
        sleep(1)
        print("sleep 1s...")

        try: geoLocation = rateLimiter((latitude, longitude), exactly_one=True, language="en")
        except Exception: return None

    if not geoLocation: return None
    address = geoLocation.raw.get("address", {})

    return AddressType({
        "id": location["id"],
        "country": address.get("country"),
        "country_code": address.get("country_code").upper(),
        "state": address.get("state"),
        "region": address.get("region").split(" ")[0],
        "country": address.get("country"),
        "city": address.get("city") or address.get("town"),
        "postcode": address.get("postcode"),
        "road": address.get("road"),
        "house_number": address.get("house_number")
    })