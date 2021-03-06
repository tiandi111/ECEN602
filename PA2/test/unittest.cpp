//
// Created by 田地 on 2021/2/28.
//

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <unordered_map>
#include "../protocol.h"

using namespace SBCP;
using MType = Message::Type;
using AType = Attribute::Type;

void testAttribute() {
    // prepare test data
    AType attrType = AType::Reason;
    std::string attrData = "tiandi";
    uint32_t totalSize = attrData.size() + 4;

    auto attr = Attribute(attrType, attrData.size(), (void *) attrData.data());

    // test getter
    assert(attrType == attr.GetType());
    assert(attrData.size() == attr.GetLen());
        assert(totalSize == attr.Size());
    assert(attrData.compare(std::string((char *) attr.PayloadPtr(), attr.GetLen())) == 0);

    // test serialization / deserialization
    char buf[attr.Size()];
    assert(attr.Size() == attr.WriteBytes((void *) buf));

    auto newAttr = Attribute((void *) buf);
    assert(attr.GetType() == newAttr.GetType());
    assert(attr.GetLen() == newAttr.GetLen());
    assert(attr.Size() == newAttr.Size());
    assert(attrData.compare(std::string((char *) newAttr.PayloadPtr(), newAttr.GetLen())) == 0);

    // test overloaded operator
    // 1. test == operator
    auto attr1 = Attribute(attrType, attrData.size(), (void *) attrData.data());
    auto attr2 = Attribute(attrType, attrData.size(), (void *) attrData.data());
    std::string attrDataReversed(attrData.rbegin(), attrData.rend());
    auto attr3 = Attribute(attrType, attrDataReversed.size(), (void *) attrDataReversed.data());
    assert(("Attribute == operator, equal case", attr1 == attr2));
    assert(("Attribute == operator, not equal case", !(attr1 == attr3)));
}

void testMessage() {
    // prepare test data
    uint16_t ver = 511;
    MType msgType = Message::JOIN;
    std::string attrData = "tiandi";

    auto msg = Message(Message::JOIN, 511, {});

    // test getter / setter
    assert(msgType == msg.GetType());
    assert(ver == msg.GetVer());
    assert(0 == msg.GetLen());

    auto attr = Attribute(Attribute::Type::Reason, attrData.size(), (void*) attrData.data());
    msg.SetAttr(attr);
    auto& attrs = msg.GetAttrMap();
    auto got = attrs.find(attr.GetType());
    assert(("Attribute should exist", got != attrs.end()));
    assert(("Message's len should equal to Attributes's total size", msg.GetLen() == got->second.Size()));

    // test overloaded operator
    // 1. test == operator
    auto msg1 = Message(Message::JOIN, 1,
        {{Attribute::Reason, Attribute(Attribute::Reason, "123")}});
    auto msg2 = Message(Message::JOIN, 1,
        {{Attribute::Reason, Attribute(Attribute::Reason, "123")}});
    assert(("Message == operator, equal case", msg1 == msg2));
    auto msg3 = Message(Message::FWD, 1,
        {{Attribute::Reason, Attribute(Attribute::Reason, "123")}});
    assert(("Message == operator, not equal case 1", !(msg1 == msg3)));
    auto msg4 = Message(Message::JOIN, 2,
                        {{Attribute::Reason, Attribute(Attribute::Reason, "123")}});
    assert(("Message == operator, not equal case 2", !(msg1 == msg4)));
    auto msg5 = Message(Message::JOIN, 1,
                        {{Attribute::Reason, Attribute(Attribute::Reason, "xxxxxx")}});
    assert(("Message == operator, not equal case 3", !(msg1 == msg5)));


    // test Read / Write
    char buf[msg.Size()];

    // 1. test constructor from data pointer
    assert(msg.WriteBytes((void *) buf) == msg.Size());
    auto newMsg = Message((void *) buf, msg.Size());
    assert(msg == newMsg);

    // 2. test ReadHeader + ReadPayload
    newMsg = Message();
    uint32_t hSize = newMsg.ReadHeader(buf);
    newMsg.ReadPayload((char *) buf + hSize);
    assert(msg == newMsg);

    // test convenient wrapper
    auto msgJoin = NewJoinMessage(attrData);
    assert(Message::JOIN == msgJoin.GetType());
    assert(msgJoin.GetAttrMap().end() != msgJoin.GetAttrMap().find(Attribute::Username));
    assert(Attribute::Username == msgJoin.GetAttrMap().at(Attribute::Username).GetType());
    assert(attrData.size() == msgJoin.GetAttrMap().at(Attribute::Username).GetLen());
    std::cout<< msgJoin <<std::endl;

}

void testIO() {
    FILE* pFile = tmpfile();

    // test message i/o
    // 1. test ReadMessage
    auto msg1 = NewJoinMessage("tiandi");
    auto msg2 = NewSendMessage("hello");

    auto f = [](Message& msg, FILE* file) {
        char buf[msg.Size()];
        assert(msg.WriteBytes((void *) buf) == msg.Size());
        assert(fwrite(buf, sizeof(char), sizeof(buf), file) == msg.Size());
    };

    f(msg1, pFile);
    f(msg2, pFile);

    assert(fseek(pFile, 0, SEEK_SET) == 0);

    Message newMsg;
    assert(ReadMessage(newMsg, fileno(pFile)) > 0);
    assert(newMsg == msg1);
    assert(ReadMessage(newMsg, fileno(pFile)) > 0);
    assert(newMsg == msg2);
}

int main() {
    testAttribute();
    testMessage();
    testIO();
}