
// cartitem.cpp
#include "CartItem.h"

CartItem::CartItem()
    : bookId(0)
    , quantity(0)
    , unitPrice(0.0)
    , discountedPrice(0.0) {
}

CartItem::CartItem(int bookId, int quantity, double unitPrice, double discountedPrice)
    : bookId(bookId)
    , quantity(quantity)
    , unitPrice(unitPrice)
    , discountedPrice(discountedPrice) {
}