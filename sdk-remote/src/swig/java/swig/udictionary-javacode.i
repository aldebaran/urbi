/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// Java code to be inserted in UDictionary generated class. It makes
// UDictionary iterable.  Inspired by
// http://chadretz.wordpress.com/2009/11/27/stl-collections-with-java-and-swig/

%typemap(javaimports) boost::unordered_map<std::string, urbi::UValue> %{
import java.util.AbstractMap;
import java.util.AbstractSet;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
%}
%typemap(javabase) boost::unordered_map<std::string, urbi::UValue> "AbstractMap<String, UValue>"
%typemap(javainterfaces) boost::unordered_map<std::string, urbi::UValue> "Map<String, UValue>"
%typemap(javacode) boost::unordered_map<std::string, urbi::UValue> %{


  private UDictionarySet dictionarySet = null;

  @Override
  public Set<Entry<String, UValue>> entrySet() {
    if (dictionarySet == null)
      dictionarySet = new UDictionarySet(this);
    return dictionarySet;
  }

  @SuppressWarnings("unchecked")
  public UValue remove(String key) {
    UValue old;
    try {
      old = get(key);
    }
    catch (RuntimeException e) {
      return null;
    }
    del(key);
    return old;
  }

  /**
   * {@inheritDoc}
   * <p>
   * Unsupported
   */
  public boolean containsValue(UValue value) {
    throw new UnsupportedOperationException();
  }

  protected class UDictionaryEntry implements Entry<String, UValue> {
    private final String key;
    private UValue value;
    private UDictionary dict;

    protected UDictionaryEntry(UDictionary d, String key, UValue value) {
      this.dict = d;
      this.key = key;
      this.value = value;
    }

    public String getKey() {
      return key;
    }

    public UValue getValue() {
      return value;
    }

    public UValue setValue(UValue value) {
      UValue old = this.value;
      this.value = value;
      put(key, value);
      return old;
    }
  }

  protected class UDictionarySet extends AbstractSet<Entry<String, UValue>>
    implements Set<Entry<String, UValue>> {

    private UDictionary dict;

    UDictionarySet(UDictionary d) {
      this.dict = d;
    }

    @Override
    public boolean add(Entry<String, UValue> item) {
      dict.put(item.getKey(), item.getValue());
      return true;
    }

    @Override
    public void clear() {
      dict.clear();
    }

    @Override
    public Iterator<Entry<String, UValue>> iterator() {
      return new UDictionarySetIterator(this.dict);
    }

    @Override
    public boolean remove(Object item) {
      return dict.remove(item) != null;
    }

    @Override
    public boolean removeAll(Collection<?> collection) {
      boolean modified = false;
      for (Object item : collection) {
	modified |= this.remove(item);
      }
      return modified;
    }

    @Override
    public boolean retainAll(Collection<?> collection) {
      //best way?
      List<Entry<String, UValue>> toRemove =
          new ArrayList<Entry<String, UValue>>(this.size());
      for (Entry<String, UValue> item : this) {
	if (!collection.contains(item)) {
	  toRemove.add(item);
	}
      }
      return removeAll(toRemove);
    }

    @Override
    public int size() {
      return dict.size();
    }
  }

  protected class UDictionarySetIterator
      implements Iterator<Entry<String, UValue>> {

    private UDictionary dict;
    private UDictionaryCPPIterator iterator;

    private UDictionarySetIterator(UDictionary d) {
      this.dict = d;
      this.iterator = d.getIterator();
    }

    public boolean hasNext() {
      return iterator.hasNext();
    }

    @SuppressWarnings("unchecked")
    public UDictionaryEntry next() {
      UDictionaryEntry res =
          new UDictionaryEntry(dict, iterator.getKey(), iterator.getValue());
      iterator.next();
      return res;
    }

    /**
     * {@inheritDoc}
     * <p>
     * Unsupported
     */
    public void remove() {
      throw new UnsupportedOperationException();
    }
  }

%}

// Local variables:
// mode: java
// End:
