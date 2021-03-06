//
// Created by 田地 on 2021/2/28.
//

#ifndef PROJ1_PROTOCOL_H
#define PROJ1_PROTOCOL_H

#include <string>
#include <unordered_map>
#include <vector>

#define PROTOCOL_VERSION 3

namespace SBCP {

/**
 * SBCP Protocol Attribute.
 */
class Attribute {

#define ATTR_HEADER_SIZE 4

  public:
    /// Attribute type enum
    enum Type {
        ST = 0,
        Reason,
        Username,
        ClientCount,
        Message,
        END,
    };

    /**
     * return if type is valid.
     * @param attribute type to evaluate.
     * @return if the given type is valid.
     */
    static bool IsValidType(Type type);

    /**
     * default constructor.
     */
    Attribute() = default;

    /**
     * construct from type, payload pointer and length.
     * @param type attribute type.
     * @param len length of the payload.
     * @param data pointer to the payload data.
     */
    Attribute(Type type, uint16_t len, void* data);

    /**
     * construct from type and payload string.
     * len will be equal to the size of the payload string.
     * @param type attribute type.
     * @param payload payload in string format.
     */
    Attribute(Type type, const std::string& payload);

    /**
     * construct from binary data.
     * header will be read first, then initialize payload according to length.
     * @param buf the pointer to byte data.
     */
    Attribute(void* buf);

    /**
     * copy constructor.
     * @param attr the attribute to copy from.
     */
    Attribute(const Attribute& attr);

    /**
     * move constructor.
     * @param attr the attribute to move from.
     */
    Attribute(Attribute&& attr);

    /**
     * write header bytes.
     * @param dst the destination.
     * @return the number of bytes written.
     */
    uint32_t WriteHeader(void* dst);

    /**
     * write payload bytes.
     * @param dst the destination.
     * @return the number of bytes written.
     */
    uint32_t WritePayload(void* dst);

    /**
     * write attribute bytes(header and payload).
     * @param dst the destination.
     * @return the number of bytes written.
     */
    uint32_t WriteBytes(void* dst);

    /**
     * read header from bytes.
     * @param src the source.
     * @return the number of bytes read.
     * @throws std::runtime_error thrown if type is invalid
     */
    uint32_t ReadHeader(void* src);

    /**
     * read payload from bytes.
     * use field `len` to indicate number of bytes to read.
     * @param src the source.
     * @return the number of bytes read.
     * @note this function must be called after attribute header is initialized
     * through `ReadHeader` or constructors.
     */
    uint32_t ReadPayload(void* src);

    /**
     * read attribute from bytes(header and payload).
     * header will be read first, then initialize payload according to length.
     * @param src the source.
     * @return the number of bytes read.
     * @throws std::runtime_error thrown if type is invalid
     */
    uint32_t ReadBytes(void* src);

    /**
     * return type.
     * @return attribute type.
     */
    Type GetType() const;

    /**
     * return len.
     * @return attribute length.
     */
    uint16_t GetLen() const;

    /**
     * return payload in char vector.
     * @return payload char vector.
     */
    std::vector<char> GetPayload() const;

    /**
     * return payload string.
     * @return payload string.
     */
    std::string GetPayloadString() const;

    /**
     * return payload pointer.
     * @return payload pointer.
     */
    void* PayloadPtr() const;

    /**
     * return the total size of the attribute.
     * @return total attribute size in bytes.
     */
    uint32_t Size() const;

    /**
     * return if `this` equals to `other`.
     * @return if attributes equal.
     */
    bool operator==(const Attribute& other) const;

private:
    // attribute header
    Type type;
    uint16_t len;

    std::vector<char> payload;
};

/**
 * output attribute stream.
 * @relatesalso Attribute.
 */
std::ostream& operator<<(std::ostream& os, const Attribute& attr);

/**
 * SBCP Protocol Message.
 */
class Message {

#define MSG_HEADER_SIZE 4

    public:
    /// Message type enum
    enum Type {
        ST = 0,
        JOIN = 2,
        FWD,
        SEND,
        END,
    };

    typedef std::unordered_map<uint16_t, Attribute> AttrMap;

    /**
     * default constructor.
     */
    Message() = default;

    /**
     * construct from fields.
     * @param type message type.
     * @param ver protocol version.
     * @param attrs message attribute map.
     */
    Message(Type type, uint16_t ver, AttrMap&& attrs);

    /**
     * construct from binary data.
     * @param src binary data pointer.
     * @param size binary data size.
     */
    Message(void* src, uint32_t size);

    /**
     * write header bytes.
     * @param dst the destination pointer.
     * @return number of bytes written.
     */
    uint32_t WriteHeader(void* dst);

    /**
     * write attributes bytes.
     * @param dst the destination.
     * @return the number of bytes written.
     */
    uint32_t WriteAttrs(void* dst);

    /**
    * write message bytes(header and attributes).
    * @param dst the destination.
    * @return the number of bytes written.
    */
    uint32_t WriteBytes(void* dst);

    /**
     * read header from bytes.
     * @param src the source.
     * @return the number of bytes read.
     * @throws std::runtime_error thrown if type is invalid
     */
    uint32_t ReadHeader(void* src);

    /**
     * read payload from bytes.
     * use field `len` to indicate number of bytes to read.
     * @param src the source.
     * @return the number of bytes read.
     *
     * @throws std::runtime_error thrown if payload data size is unmatched with
     * field `len` .
     * @throws @see `ReadHeader` method of Attribute class.
     *
     * @note this function must be called after message header is initialized
     * through `ReadHeader` or constructors.
     */
    uint32_t ReadPayload(void* src);

    /**
     * read message from bytes(header and payload).
     * header will be read first, then initialize payload according to field `len'.
     * @param src the source.
     * @return the number of bytes read.
     *
     * @throws @see `ReadHeader` and `ReadPayload`.
     */
    ssize_t ReadBytes(void* src, uint32_t size);

    /**
     * return message protocol version.
     * @return ver.
     */
    uint32_t GetVer() const;

    /**
     * return message type.
     * @return type.
     */
    uint32_t GetType() const;

    /**
     * return message payload size.
     * @return len.
     */
    uint32_t GetLen() const;

    /**
     * return message attribute map.
     * @return attrMap.
     */
    const AttrMap& GetAttrMap() const;

    /**
     * return message total size
     * @return message total size.
     */
    uint32_t Size() const;

    /**
     * set an attribute to attribute map.
     * if the attribute type already exits, replace it with the given attribute.
     * @return message total size.
     */
    void SetAttr(Attribute attr);

    /**
     * return if `this` equals to `other`.
     * @return if messages equal.
     */
    bool operator==(const Message& other);

  private:
    // header
    Type type;
    uint16_t ver;
    uint16_t len;
    // attribute map, key is attribute type and value is attribute itself
    AttrMap attrMap;
};

/**
 * output message stream.
 * @relatesalso Message.
 */
std::ostream& operator<<(std::ostream& os, const Message& msg);

/**
 * return a new message with type JOIN.
 * @param username the username attribute.
 * @return a JOIN message.
 *
 * @relatesalso Message.
 */
Message NewJoinMessage(const std::string& username);

/**
 * return a new message with type FWD.
 * @param username the payload of username attribute.
 * @param payload the payload of message attribute.
 * @return a FWD message.
 *
 * @relatesalso Message.
 */
Message NewFWDMessage(const std::string& username, const std::string& message);

/**
 * return a new message with type FWD.
 * @param username the username attribute.
 * @return a SEND message.
 *
 * @relatesalso Message.
 */
Message NewSendMessage(const std::string& payload);

/**
 * read a message from file descriptor
 * @param msg the message .
 * @param fd the file descriptor.
 * @return the number of bytes read if succeeds, -1 if error occurred,
 * 0 if end of file occurred(if fd is a socket, 0 indicates the close of the connection).
 *
 * @relatesalso Message.
 */
ssize_t ReadMessage(Message& msg, int fd);

}

#endif //PROJ1_PROTOCOL_H
