from configs import (
    POSTGRE,

    QUERY_SELECT_LOCATION,
    QUERY_INSERT_ADDRESS,
    # QUERY_CREATE_ADDRESS
)
from utils import findAddress

def main():
    # POSTGRE.createTable(QUERY_CREATE_ADDRESS)
    data = POSTGRE.select(QUERY_SELECT_LOCATION)

    listAddresses = []

    for row in data:
        address = findAddress(row)
        addressParams = tuple(address.values())

        listAddresses.append(addressParams)

    POSTGRE.batchInsert(QUERY_INSERT_ADDRESS, listAddresses)

if __name__ == "__main__": main()