package appView;

public interface SettingListener
{
	default void onChanged(String name, String oldValue, String newValue)
	{}
}
